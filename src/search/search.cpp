/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <QApplication>
#include <QFlags>
#include <QTextDocument>
#include <QMessageBox>
#include <QPushButton>
#include "search.h"
#include "searchdialog.h"
#include "searchworker.h"
#include "exception.h"
#include "editors/abstractedit.h"
#include "editors/textview.h"
#include "viewhelper.h"

namespace gams {
namespace studio {
namespace search {

Search::Search(MainWindow *main) : mMain(main)
{ }

void Search::setParameters(QList<FileMeta*> files, QRegularExpression regex, bool searchBackwards)
{
    mFiles = files;
    mRegex = regex;
    mOptions = QFlags<QTextDocument::FindFlag>();

    // this is needed for document->find as that is not using the regexes case sensitivity setting
    mOptions.setFlag(QTextDocument::FindCaseSensitively,
                     !mRegex.patternOptions().testFlag(QRegularExpression::CaseInsensitiveOption));
    mOptions.setFlag(QTextDocument::FindBackward, searchBackwards);
}

void Search::start()
{
    if (mSearching || mRegex.pattern().isEmpty()) return;
    mResults.clear();
    mResultHash.clear();
    mSearching = true;

    if (mMain->searchDialog()->selectedScope() == Scope::Selection) {
        findInSelection();
        return;
    } // else:

    QList<FileMeta*> unmodified;
    QList<FileMeta*> modified; // need to be treated differently

    for(FileMeta* fm : qAsConst(mFiles)) {
        // skip certain file types
        if (fm->kind() == FileKind::Gdx || fm->kind() == FileKind::Ref)
            continue;

        // sort files by modified
        if (fm->isModified()) modified << fm;
        else unmodified << fm;
    }

    // non-parallel first
    for (FileMeta* fm : qAsConst(modified))
        findInDoc(fm);

    SearchWorker* sw = new SearchWorker(unmodified, mRegex, &mResults);
    sw->moveToThread(&mThread);

    connect(&mThread, &QThread::finished, sw, &QObject::deleteLater, Qt::UniqueConnection);
    connect(&mThread, &QThread::finished, this, &Search::finished, Qt::UniqueConnection);
    connect(&mThread, &QThread::started, sw, &SearchWorker::findInFiles, Qt::UniqueConnection);
    connect(sw, &SearchWorker::update, mMain->searchDialog(), &SearchDialog::intermediateUpdate, Qt::UniqueConnection);

    mThread.start();
    mThread.setPriority(QThread::LowPriority); // search is a background task
}

void Search::stop()
{
    mThread.requestInterruption();
}

void Search::resetResults()
{
    mFiles.clear();
    mResults.clear();
    mResultHash.clear();

    if (mMain->searchDialog())
        mMain->searchDialog()->updateEditHighlighting();
}

void Search::reset()
{
    resetResults();

    mCacheAvailable = false;
    mOutsideOfList = false;
    mLastMatchInOpt = -1;

    mThread.isInterruptionRequested();
    mMain->invalidateResultsView();
}

void Search::documentChanged()
{
    mMain->invalidateResultsView();
    mCacheAvailable = false;
}

void Search::findInSelection()
{
    if (AbstractEdit* ae = ViewHelper::toAbstractEdit(mMain->recent()->editor())) {
        checkFileChanged(ae->fileId());
        ae->findInSelection(mResults);
    } else if (TextView* tv = ViewHelper::toTextView(mMain->recent()->editor())) {
        checkFileChanged(tv->edit()->fileId());
        tv->findInSelection(mRegex, mMain->fileRepo()->fileMeta(mSearchSelectionFile), &mResults);
    }
    mMain->searchDialog()->updateClearButton();

    // nothing more to do, update UI and return
    finished();
}

void Search::checkFileChanged(FileId fileId) {
    if (mSearchSelectionFile != fileId) {
        mMain->invalidateResultsView();
        mSearchSelectionFile = fileId;
    }
}

void Search::findInDoc(FileMeta* fm)
{
    QTextCursor item;
    QTextCursor lastItem = QTextCursor(fm->document());

    // ignore search direction for cache generation. otherwise results would be in wrong order
    QFlags<QTextDocument::FindFlag> cacheOptions = mOptions;
    cacheOptions.setFlag(QTextDocument::FindBackward, false);

    do {
        item = fm->document()->find(mRegex, lastItem, cacheOptions);
        if (item != lastItem) lastItem = item;
        else break;

        if (!item.isNull()) {
            mResults.append(Result(item.blockNumber()+1, item.positionInBlock() - item.selectedText().length(),
                                   item.selectedText().length(), fm->location(), item.block().text().trimmed()));
        }
        if (mResults.size() > MAX_SEARCH_RESULTS) break;
    } while (!item.isNull());
}

void Search::findNext(Direction direction)
{
    // create new cache when cached search does not contain results for current file
    QString location = mMain->fileRepo()->fileMeta(mMain->recent()->editor())->location();
    bool requestNewCache = !mCacheAvailable || !hasResultsForFile(location);

    if (requestNewCache) {
        mMain->invalidateResultsView();
        mCacheAvailable = false;
        mMain->searchDialog()->updateUi(true);
        if (mMain->resultsView()) mMain->resultsView()->setOutdated();
        start();
    }
    selectNextMatch(direction);
}

///
/// \brief Search::cursorPosition helper function to get cursor from all editor types that have one
/// \return QPair with lineNr, colNr
///
QPair<int, int> Search::cursorPosition() {
    int lineNr = 0;
    int colNr = 0;

    if (AbstractEdit* e = ViewHelper::toAbstractEdit(mMain->recent()->editor())) {
        lineNr = e->textCursor().blockNumber()+1;
        colNr = e->textCursor().positionInBlock();
    } else if (TextView* t = ViewHelper::toTextView(mMain->recent()->editor())) {
        lineNr = t->position().y()+1;
        colNr = t->position().x();
    }
    return QPair<int, int>(lineNr, colNr);
}

///
/// \brief Search::findNextEntryInCache compares cursor position to list of results to find the next match.
/// \param direction Search::Direction
/// \param cursorPos QPair of LineNr and ColumnNr
/// \return index of match in result list
///
int Search::findNextEntryInCache(Search::Direction direction) {
    QPair<int, int> cursorPos = cursorPosition();

    int start = direction == Direction::Backward ? mResults.size()-1 : 0;
    int iterator = direction == Direction::Backward ? -1 : 1;
    bool allowJumping = false;

    // allow jumping when we have results but not in the current file
    allowJumping = (mResults.size() > 0)
            && hasResultsForFile(mMain->fileRepo()->fileMeta(mMain->recent()->editor())->location());

    // TODO(RG): refactoring candidate:
    if (mMain->recent()->editor()) {
        QString file = ViewHelper::location(mMain->recent()->editor());
        for (int i = start; i >= 0 && i < mResults.size(); i += iterator) {
            Result r = mResults.at(i);

            // check if is in same line but behind the cursor
            if (file == r.filepath()) {
                allowJumping = true; // allow jumping after searching the current file

                // just jump to next result if in Solver Option Editor
                if (ViewHelper::toSolverOptionEdit(mMain->recent()->editor())) {
                    if (direction == Direction::Forward) {
                        if (i < mLastMatchInOpt)
                            continue; // catch up with last hit in opt file
                    } else { // if backwards
                        if (mLastMatchInOpt == 0) mLastMatchInOpt = mResults.size();
                        if (i > mLastMatchInOpt)
                            continue;
                    }

                    // select next
                    int newIndex = i + iterator;
                    if (newIndex < 0)
                        newIndex = mResults.size()-1;
                    else if (newIndex > mResults.size()-1)
                        newIndex = 0;

                    mLastMatchInOpt = newIndex;
                    return newIndex;
                }

                if (direction == Direction::Backward) {
                    if (cursorPos.first > r.lineNr() || (cursorPos.first == r.lineNr() && cursorPos.second > r.colNr() + r.length())) {
                        return (i == MAX_SEARCH_RESULTS) ? -1 : i;
                    }
                } else {
                    if (cursorPos.first < r.lineNr() || (cursorPos.first == r.lineNr() && cursorPos.second <= r.colNr())) {
                        return (i == MAX_SEARCH_RESULTS) ? -1 : i;
                    }
                }
            } else if (file != r.filepath() && allowJumping) {
                // first match in next file
                return i;
            }
        }
    }
    return -1; // not found
}

/// Does a check if matchNr is valid and jumps to the corresponding result
/// \brief Search::jumpToResult
/// \param matchNr index of match in cache
///
void Search::jumpToResult(int matchNr)
{
    if (matchNr > -1 && matchNr < mResults.size()) {
        PExFileNode *node = mMain->projectRepo()->findFile(mResults.at(matchNr).filepath());
        if (!node) EXCEPT() << "File not found: " << mResults.at(matchNr).filepath();

        node->file()->jumpTo(node->projectId(), true, mResults.at(matchNr).lineNr()-1,
                             qMax(mResults.at(matchNr).colNr(), 0), mResults.at(matchNr).length());
    }
}

///
/// \brief SearchDialog::selectNextMatch steps through words in a document
/// \param direction
///
void Search::selectNextMatch(Direction direction, bool firstLevel)
{
    int matchNr = -1;

    if (mCacheAvailable && !mOutsideOfList)
        matchNr = NavigateInsideCache(direction);

    // dont jump outside of cache when searchscope is set to selection
    if ((mOutsideOfList || !mCacheAvailable) && mMain->searchDialog()->selectedScope() != Scope::Selection)
        matchNr = NavigateOutsideCache(direction, firstLevel);

    // update ui
    mMain->searchDialog()->updateNrMatches(matchNr+1);
    if (mMain->resultsView() && !mMain->resultsView()->isOutdated() && (!mOutsideOfList || matchNr == -1))
        mMain->resultsView()->selectItem(matchNr);
}


int Search::NavigateOutsideCache(Direction direction, bool firstLevel)
{
    int matchNr = -1;
    bool found = false;

    if (AbstractEdit* e = ViewHelper::toAbstractEdit(mMain->recent()->editor())) {

        QTextCursor tc = e->textCursor();
        if (!firstLevel) {
            if (direction == Direction::Backward) tc.movePosition(QTextCursor::End);
            else tc.movePosition(QTextCursor::Start);
        }

        QTextCursor ntc = e->document()->find(mRegex, direction == Direction::Backward
                                             ? tc.position()-tc.selectedText().length()
                                             : tc.position(), mOptions);
        found = !ntc.isNull();
        if (found) {
            e->jumpTo(ntc.blockNumber()+1, ntc.columnNumber());
            e->setTextCursor(ntc);
        }

    } else if (TextView* t = ViewHelper::toTextView(mMain->recent()->editor())) {
        mSplitSearchContinue = !firstLevel;
        found = t->findText(mRegex, mOptions, mSplitSearchContinue);
    }

    // try again
    if (!found) {
        matchNr = findNextEntryInCache(direction);
        found = matchNr != -1;
        jumpToResult(matchNr);
    }

    if (!found && firstLevel) selectNextMatch(direction, false);

    // check if cache was re-entered
    matchNr = mMain->searchDialog()->updateLabelByCursorPos();
    mOutsideOfList = matchNr == -1;

    return matchNr;
}

int Search::NavigateInsideCache(Direction direction)
{
    int matchNr = findNextEntryInCache(direction);

     // nothing found
    if (matchNr == -1) {
        // check if we should leave cache navigation
        mOutsideOfList = mResults.size() >= MAX_SEARCH_RESULTS; // now leaving cache
        // if not, jump to start/end
        if (!mOutsideOfList && mResults.size() > 0) {
            matchNr = (direction == Direction::Backward) ? mResults.size()-1 : 0;
            mLastMatchInOpt = matchNr;
        }
    } else {
        mOutsideOfList = false;
    }
    // navigate to match
    jumpToResult(matchNr);

    return matchNr;
}

bool Search::hasResultsForFile(QString filePath) {
    QHash<QString,QList<Result>>::iterator results = mResultHash.find(filePath);
    if (results == mResultHash.end())
        return false;
    else
        return results->count() > 0;
}

///
/// \brief SearchDialog::replaceUnopened replaces in files where there is currently no editor open
/// \param fm file
/// \param regex find
/// \param replaceTerm replace with
///
int Search::replaceUnopened(FileMeta* fm, QRegularExpression regex, QString replaceTerm)
{
    QFile file(fm->location());
    QTextStream ts(&file);
    ts.setCodec(fm->codec());
    int hits = 0;

    if (file.open(QIODevice::ReadWrite)) {
        QString content = ts.readAll();
        hits = content.count(regex);
        content.replace(regex, replaceTerm);

        ts.seek(0);
        ts << content;
    }
    file.close();
    return hits;
}
 ///
/// \brief SearchDialog::replaceOpened uses QTextDocument for replacing strings. this allows the user
/// to undo changes made by replacing.
/// \param fm filemeta
/// \param regex find
/// \param replaceTerm replace with
/// \param flags options
///
int Search::replaceOpened(FileMeta* fm, QRegularExpression regex, QString replaceTerm)
{
    AbstractEdit* ae = ViewHelper::toAbstractEdit(fm->editors().constFirst());

    int hits = 0;
    if (ae && fm->editors().size() > 0) {
        hits = ae->replaceAll(fm, regex, replaceTerm, mOptions,
                              mMain->searchDialog()->selectedScope() == Scope::Selection);
    }
    return hits;
}

void Search::finished()
{
    mSearching = false;

    for (const Result &r : qAsConst(mResults))
        mResultHash[r.filepath()].append(r);

    mCacheAvailable = true;
    mMain->searchDialog()->finalUpdate();
}

const QFlags<QTextDocument::FindFlag> &Search::options() const
{
    return mOptions;
}

bool Search::hasSearchSelection()
{
    if (mMain->searchDialog()->selectedScope() != Scope::Selection) return false;

    if (AbstractEdit *ce = ViewHelper::toAbstractEdit(mMain->recent()->editor())) {
        return ce->hasSearchSelection();
    } else if (TextView *tv = ViewHelper::toTextView(mMain->recent()->editor())) {
        return tv->edit()->hasSearchSelection();
    }
    return false;
}

QRegularExpression Search::regex() const
{
    return mRegex;
}

bool Search::isRunning() const
{
    return mSearching;
}

QList<Result> Search::results() const
{
    return mResults;
}

QList<Result> Search::filteredResultList(QString fileLocation)
{
    return mResultHash[fileLocation];
}

void Search::replaceNext(QString replacementText)
{
    AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
    if (!edit) return;

    edit->replaceNext(mRegex, replacementText, mMain->searchDialog()->selectedScope() == Search::Selection);

    start(); // refresh cache
    selectNextMatch();
}

void Search::replaceAll(QString replacementText)
{
    if (mRegex.pattern().isEmpty()) return;

    QList<FileMeta*> opened;
    QList<FileMeta*> unopened;

    int matchedFiles = 0;

    // sort and filter FMs by editability and modification state
    for (FileMeta* fm : qAsConst(mFiles)) {
        if (fm->isReadOnly()) {
            mFiles.removeOne(fm);
            continue;
        }

        if (fm->document()) {
            if (!opened.contains(fm)) opened << fm;
        } else {
            if (!unopened.contains(fm)) unopened << fm;
        }

        matchedFiles++;
    }

    // user interaction
    QString searchTerm = mRegex.pattern();
    QString replaceTerm = replacementText;
    QMessageBox msgBox;
    if (mFiles.length() == 0) {
        msgBox.setText("No files matching criteria.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;

    } else if (matchedFiles == 1 && mMain->searchDialog()->selectedScope() != Search::Selection) {
        msgBox.setText("Are you sure you want to replace all occurrences of '" + searchTerm
                       + "' with '" + replaceTerm + "' in file " + mFiles.first()->name() + "?");
    } else if (mMain->searchDialog()->selectedScope() == Search::Selection) {
        msgBox.setText("Are you sure you want to replace all occurrences of '" + searchTerm
                       + "' with '" + replaceTerm + "' in the selected text in file "
                       + mFiles.first()->name() + "?");
    } else {
        msgBox.setText("Are you sure you want to replace all occurrences of '" +
                       searchTerm + "' with '" + replaceTerm + "' in " + QString::number(matchedFiles) + " files? " +
                       "This action cannot be undone!");
        QString detailedText;
        msgBox.setInformativeText("Click \"Show Details...\" to show selected files.");

        for (FileMeta* fm : qAsConst(mFiles))
            detailedText.append(fm->location()+"\n");
        detailedText.append("\nThese files do not necessarily have any matches in them. "
                            "This is just a representation of the selected scope in the search window. "
                            "Press \"Search\" to see actual matches that will be replaced.");
        msgBox.setDetailedText(detailedText);
    }
    QPushButton *ok = msgBox.addButton(QMessageBox::Ok);
    QPushButton *cancel = msgBox.addButton(QMessageBox::Cancel);
    QPushButton *preview = msgBox.addButton("Preview", QMessageBox::RejectRole);
    msgBox.setDefaultButton(preview);

    int hits = 0;
    msgBox.exec();
    if (msgBox.clickedButton() == ok) {

        mMain->searchDialog()->setSearchStatus(Search::Replacing);
        QApplication::processEvents(QEventLoop::AllEvents, 10); // to show change in UI

        for (FileMeta* fm : qAsConst(opened))
            hits += replaceOpened(fm, mRegex, replaceTerm);

        for (FileMeta* fm : qAsConst(unopened))
            hits += replaceUnopened(fm, mRegex, replaceTerm);

        mMain->searchDialog()->searchParameterChanged();
    } else if (msgBox.clickedButton() == preview) {
        start();
        return;
    } else if (msgBox.clickedButton() == cancel) {
        return;
    }

    QMessageBox ansBox;
    if (hits == 1)
        ansBox.setText("1 occurrences of '" + searchTerm + "' was replaced with '" + replaceTerm + "'.");
    else
        ansBox.setText(QString::number(hits) + " occurrences of '" + searchTerm
                        + "' were replaced with '" + replaceTerm + "'.");
    ansBox.addButton(QMessageBox::Ok);
    ansBox.exec();
}

}
}
}
