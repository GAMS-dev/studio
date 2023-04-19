/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include <QFileDialog>
#include "search.h"
#include "searchdialog.h"
#include "searchworker.h"
#include "editors/abstractedit.h"
#include "editors/textview.h"
#include "editors/sysloglocator.h"
#include "viewhelper.h"

namespace gams {
namespace studio {
namespace search {

Search::Search(SearchDialog *sd, AbstractSearchFileHandler* fileHandler) : mSearchDialog(sd), mFileHandler(fileHandler)
{
    connect(this, &Search::invalidateResults, mSearchDialog, &SearchDialog::invalidateResultsView);
    connect(this, &Search::selectResult, mSearchDialog, &SearchDialog::selectResult);
}

void Search::setParameters(bool ignoreReadonly, bool searchBackwards)
{
    mFiles = mSearchDialog->getFilesByScope(ignoreReadonly);
    mRegex = mSearchDialog->createRegex();
    mSearchTerm = mSearchDialog->searchTerm();
    mOptions = QFlags<QTextDocument::FindFlag>();

    // this is needed for document->find as that is not using the regexes case sensitivity setting:
    mOptions.setFlag(QTextDocument::FindCaseSensitively,
                     !mRegex.patternOptions().testFlag(QRegularExpression::CaseInsensitiveOption));
    mOptions.setFlag(QTextDocument::FindBackward, searchBackwards);
    mScope = mSearchDialog->selectedScope();
    mSearchDialog->setSearchedFiles(mFiles.size());
}

void Search::start(bool ignoreReadonly, bool searchBackwards, bool showResults)
{
    if (mSearching) return;
    // setup
    mResults.clear();
    mResultHash.clear();

    mSearching = true;
    mSearchDialog->setSearchStatus(Search::CollectingFiles);
    mSearchDialog->updateDialogState();

    setParameters(ignoreReadonly, searchBackwards);
    if (mRegex.pattern().isEmpty()) {
        mSearchDialog->setSearchStatus(Search::Clear);
        mSearchDialog->finalUpdate();
        return;
    }

    if (mSearchDialog->selectedScope() == Scope::Selection) {
        findInSelection(showResults);
        return;
    } // else
    QList<FileMeta*> unmodified;
    QList<FileMeta*> modified; // need to be treated differently

    FileMeta* currentFile = mSearchDialog->fileHandler()->fileMeta(mSearchDialog->currentEditor());
    for(FileMeta* fm : qAsConst(mFiles)) {
        // skip certain file types
        if (fm->kind() == FileKind::Gdx || fm->kind() == FileKind::Ref)
            continue;

        // sort files by modified, current file first
        if (fm->isModified() && fm->document()) {
            if (fm == currentFile) modified.insert(0, fm);
            else modified << fm;
        } else {
            if (fm == currentFile) unmodified.insert(0, fm);
            else unmodified << fm;
        }
    }
    // start background task first
    NodeId projectNode = mSearchDialog->selectedScope() == Scope::ThisProject ? currentFile->projectId() : NodeId();
    SearchWorker* sw = new SearchWorker(unmodified, mRegex, &mResults, projectNode, showResults);
    sw->moveToThread(&mThread);

    connect(&mThread, &QThread::finished, sw, &QObject::deleteLater, Qt::UniqueConnection);
    connect(&mThread, &QThread::finished, this, &Search::finished, Qt::UniqueConnection);
    connect(&mThread, &QThread::started, sw, &SearchWorker::findInFiles, Qt::UniqueConnection);
    connect(sw, &SearchWorker::update, mSearchDialog, &SearchDialog::intermediateUpdate, Qt::UniqueConnection);
    connect(sw, &SearchWorker::showResults, mSearchDialog, &SearchDialog::relaySearchResults, Qt::UniqueConnection);

    mThread.start();
    mThread.setPriority(QThread::LowPriority); // search is a background task

    for (FileMeta* fm : qAsConst(modified))
        findInDoc(fm);
}

void Search::stop()
{
    mThread.requestInterruption();
}

void Search::activeFileChanged()
{
    if (!mFiles.contains(mSearchDialog->fileHandler()->fileMeta(mSearchDialog->currentEditor()))){
        mCacheAvailable = false;
    }
}

void Search::reset()
{
    resetResults();

    invalidateCache();
    mOutsideOfList = false;
    mLastMatchInOpt = -1;
    mSearchDialog->setSearchStatus(Status::Clear);

    mFiles.clear();
    mRegex = QRegularExpression("");
    mOptions = QFlags<QTextDocument::FindFlag>();
    mSearchDialog->setSearchedFiles(0);

    mThread.isInterruptionRequested();
    emit invalidateResults();
}

void Search::resetResults()
{
    mFiles.clear();
    mResults.clear();
    mResultHash.clear();

    if (mSearchDialog)
        mSearchDialog->updateEditHighlighting();
}

void Search::invalidateCache()
{
    emit invalidateResults();
    mCacheAvailable = false;
}

void Search::findInSelection(bool showResults)
{
    if (AbstractEdit* ae = ViewHelper::toAbstractEdit(mSearchDialog->currentEditor())) {
        checkFileChanged(ae->fileId());
        ae->findInSelection(mResults);
        mSearchDialog->relaySearchResults(showResults, &mResults);
    } else if (TextView* tv = ViewHelper::toTextView(mSearchDialog->currentEditor())) {
        checkFileChanged(tv->edit()->fileId());
        tv->findInSelection(mRegex, mSearchDialog->fileHandler()->fileMeta(mSearchSelectionFile), &mResults, showResults);
    }
    mSearchDialog->updateClearButton();

    // nothing more to do, update UI and return
    finished();
}

void Search::checkFileChanged(FileId fileId) {
    if (mSearchSelectionFile != fileId) {
        emit invalidateResults();
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
                                   item.selectedText().length(), fm->location(),
                                   fm->projectId(), item.block().text()));
        }
        if (mResults.size() > MAX_SEARCH_RESULTS) break;
    } while (!item.isNull());
}

void Search::findNext(Direction direction)
{
    if (!mCacheAvailable) {
        emit invalidateResults();
        emit updateUI();
        start(false, direction == Search::Backward, false); // generate new cache
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

    if (AbstractEdit* e = ViewHelper::toAbstractEdit(mSearchDialog->currentEditor())) {
        lineNr = e->textCursor().blockNumber()+1;
        colNr = e->textCursor().positionInBlock();
    } else if (TextView* t = ViewHelper::toTextView(mSearchDialog->currentEditor())) {
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

    FileMeta* currentFm = mSearchDialog->fileHandler()->fileMeta(mSearchDialog->currentEditor());

    // allow jumping when we have results but not in the current file
    bool allowJumping = (mResults.size() > 0)
            && !hasResultsForFile(currentFm ? currentFm->location() : "");

    if (mSearchDialog->currentEditor()) {
        QString file = ViewHelper::location(mSearchDialog->currentEditor());
        for (int i = start; i >= 0 && i < mResults.size(); i += iterator) {
            Result r = mResults.at(i);

            // check if is in same line but behind the cursor
            if (file == r.filepath()) {
                allowJumping = true; // allow jumping after searching the current file

                // just jump to next result if in Solver Option Editor
                if (ViewHelper::toSolverOptionEdit(mSearchDialog->currentEditor())) {
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
    mSearchDialog->jumpToResult(matchNr);
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

    // queue jump for when cache is finished. will be reset if an outside-of-cache result is found
    if (mSearching) mJumpQueued = true;

    // dont jump outside of cache when searchscope is set to selection
    if ((mOutsideOfList || !mCacheAvailable) && mSearchDialog->selectedScope() != Scope::Selection)
        matchNr = NavigateOutsideCache(direction, firstLevel);

    // update ui
    mSearchDialog->updateMatchLabel(matchNr+1);
    if (!mOutsideOfList || matchNr == -1)
        emit selectResult(matchNr);
}


int Search::NavigateOutsideCache(Direction direction, bool firstLevel)
{
    int matchNr = -1;
    bool found = false;

    // do not move cursor in document that is not in scope:
    if (mSearchDialog->getFilesByScope().contains(mSearchDialog->fileHandler()->fileMeta(mSearchDialog->currentEditor()))) {
        if (AbstractEdit* e = ViewHelper::toAbstractEdit(mSearchDialog->currentEditor())) {

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

        } else if (TextView* t = ViewHelper::toTextView(mSearchDialog->currentEditor())) {
            mSplitSearchContinue = !firstLevel;
            found = t->findText(mRegex, mOptions, mSplitSearchContinue);
        }
    }

    // try again
    if (!found && !mSearching) {
        matchNr = findNextEntryInCache(direction);
        found = matchNr != -1;

        jumpToResult(matchNr);
    }

    if (!found && firstLevel) selectNextMatch(direction, false);
    if (found) mJumpQueued = false;

    // check if cache was re-entered
    matchNr = mSearchDialog->updateLabelByCursorPos();
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
    QTextCodec *codec = fm->codec();
    int hits = 0;

    if (!file.open(QFile::ReadWrite)) {
        SysLogLocator::systemLog()->append("Unable to open file " + fm->location(), LogMsgType::Error);
        return 0;
    }

    QString content;
    while (!file.atEnd()) {
        QByteArray arry = file.readLine();
        QString line = codec ? codec->toUnicode(arry) : QString(arry);

        if (regex.captureCount() > 0) {
            QRegularExpressionMatchIterator matchIter = regex.globalMatch(line);

            // iterate over matches in this line
            while(matchIter.hasNext()) {
                QRegularExpressionMatch match = matchIter.next();
                QString modifiedReplaceTerm = replaceTerm;

                // replace capture groups placeholders with content
                for(int i = 1; i <= regex.captureCount(); i++) {
                    QRegularExpression replaceGroup("\\$" + QString::number(i));
                    QString captured = match.capturedTexts().at(i);
                    modifiedReplaceTerm.replace(replaceGroup, captured);
                }

                // replace match with filled replace term
                line.replace(match.capturedStart(), match.capturedLength(), modifiedReplaceTerm);
                hits++;
            }
        } else {
            hits += line.count(regex);
            line.replace(regex, replaceTerm);
        }
        content += line + "\n";
    }

    file.seek(0);
    file.write(codec ? codec->fromUnicode(content) : content.toUtf8());
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
                              mSearchDialog->selectedScope() == Scope::Selection);
    }
    return hits;
}

void Search::finished()
{
    mSearching = false;

    for (const Result &r : qAsConst(mResults))
        mResultHash[r.filepath()].append(r);

    mCacheAvailable = mResults.count() > 0;
    mSearchDialog->finalUpdate();

    if (mJumpQueued) {
        mJumpQueued = false;
        selectNextMatch();
    }
}

bool Search::hasCache() const
{
    return mCacheAvailable;
}

Search::Scope Search::scope() const
{
    return mScope;
}

const QFlags<QTextDocument::FindFlag> &Search::options() const
{
    return mOptions;
}

bool Search::hasSearchSelection()
{
    if (AbstractEdit *ce = ViewHelper::toAbstractEdit(mSearchDialog->currentEditor())) {
        return ce->hasSearchSelection();
    } else if (TextView *tv = ViewHelper::toTextView(mSearchDialog->currentEditor())) {
        return tv->edit()->hasSearchSelection();
    }
    return false;
}

const QRegularExpression Search::regex() const
{
    return mRegex;
}

bool Search::isSearching() const
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
    AbstractEdit* edit = ViewHelper::toAbstractEdit(mSearchDialog->currentEditor());
    if (!edit) return;

    edit->replaceNext(mRegex, replacementText, mSearchDialog->selectedScope() == Search::Selection);

    start(true, false, false); // refresh cache
    selectNextMatch();
}

void Search::replaceAll(QString replacementText)
{
    setParameters(true);

    if (mRegex.pattern().isEmpty()) return;

    QList<FileMeta*> opened;
    QList<FileMeta*> unopened;

    int matchedFiles = 0;

    QList<FileMeta*> searchFiles;

    // sort and filter FMs by editability and modification state
    for (FileMeta* fm : qAsConst(mFiles)) {
        if (!fm->isReadOnly()) {
            searchFiles.append(fm);
        } else continue;

        if (fm->document()) {
            if (!opened.contains(fm)) opened << fm;
        } else {
            if (!unopened.contains(fm)) unopened << fm;
        }

        matchedFiles++;
    }

    // user interaction
    QString replaceTerm = replacementText;
    QMessageBox msgBox;
    if (mFiles.size() == 0) {
        msgBox.setText("No files matching criteria.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;

    } else if (matchedFiles == 1 && mSearchDialog->selectedScope() != Search::Selection) {
        msgBox.setText("Are you sure you want to replace all occurrences of '" + mSearchTerm
                       + "' with '" + replaceTerm + "' in file " + searchFiles.first()->name() + "?");
    } else if (mSearchDialog->selectedScope() == Search::Selection) {
        msgBox.setText("Are you sure you want to replace all occurrences of '" + mSearchTerm
                       + "' with '" + replaceTerm + "' in the selected text in file "
                       + searchFiles.first()->name() + "?");
    } else {
        msgBox.setText("Are you sure you want to replace all occurrences of '" +
                       mSearchTerm + "' with '" + replaceTerm + "' in " + QString::number(matchedFiles) + " files? " +
                       "This action cannot be undone!");
        QString detailedText;
        msgBox.setInformativeText("Click \"Show Details...\" to show selected files.");

        for (FileMeta* fm : qAsConst(searchFiles))
            detailedText.append(fm->location()+"\n");
        detailedText.append("\nThese files do not necessarily have any matches in them. "
                            "This is just a representation of the selected scope in the search window. "
                            "Press \"Preview\" to see actual matches that will be replaced.");
        msgBox.setDetailedText(detailedText);
    }
    QPushButton *ok = msgBox.addButton(QMessageBox::Ok);
    QPushButton *cancel = msgBox.addButton(QMessageBox::Cancel);
    QPushButton *preview = msgBox.addButton("Preview", QMessageBox::RejectRole);
    msgBox.setDefaultButton(preview);

    int hits = 0;
    msgBox.exec();
    if (msgBox.clickedButton() == ok) {

        mSearchDialog->setSearchStatus(Search::Replacing);
        setParameters(true);

        for (FileMeta* fm : qAsConst(opened))
            hits += replaceOpened(fm, mRegex, replaceTerm);

        for (FileMeta* fm : qAsConst(unopened))
            hits += replaceUnopened(fm, mRegex, replaceTerm);

        mSearchDialog->searchParameterChanged();
    } else if (msgBox.clickedButton() == preview) {
        start(true, false, true);
        return;
    } else if (msgBox.clickedButton() == cancel) {
        return;
    }

    QMessageBox ansBox;
    if (hits == 1)
        ansBox.setText("1 occurrences of '" + mSearchTerm + "' was replaced with '" + replaceTerm + "'.");
    else
        ansBox.setText(QString::number(hits) + " occurrences of '" + mSearchTerm
                        + "' were replaced with '" + replaceTerm + "'.");
    ansBox.addButton(QMessageBox::Ok);
    ansBox.exec();
}

}
}
}
