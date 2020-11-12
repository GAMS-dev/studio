/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "search.h"
#include "searchdialog.h"
#include "searchworker.h"
#include "exception.h"

#include <QApplication>
#include <QFlags>
#include <QTextDocument>
#include <QMessageBox>
#include <QPushButton>
#include <viewhelper.h>

namespace gams {
namespace studio {
namespace search {

Search::Search(MainWindow *main) : mMain(main)
{

}

void Search::setParameters(QList<FileMeta*> files, QRegularExpression regex, bool searchBackwards)
{
    mFiles = files;
    mRegex = regex;
    mOptions = QFlags<QTextDocument::FindFlag>();

    mOptions.setFlag(QTextDocument::FindBackward, searchBackwards);
}

void Search::start()
{
    if (mSearching || mRegex.pattern().isEmpty()) return;

    mResults.clear();
    mResultHash.clear();

    mSearching = true;
    QList<FileMeta*> unmodified;
    QList<FileMeta*> modified; // need to be treated differently

    for(FileMeta* fm : mFiles) {
        // skip certain file types
        if (fm->kind() == FileKind::Gdx || fm->kind() == FileKind::Ref)
            continue;

        // sort files by modified
        if (fm->isModified()) modified << fm;
        else unmodified << fm;
    }

    // non-parallel first
    for (FileMeta* fm : modified)
        findInDoc(fm);

    SearchWorker* sw = new SearchWorker(unmodified, mRegex, &mResults);
    sw->moveToThread(&mThread);

    connect(&mThread, &QThread::finished, sw, &QObject::deleteLater, Qt::UniqueConnection);
    connect(&mThread, &QThread::finished, this, &Search::finished, Qt::UniqueConnection);
    connect(&mThread, &QThread::started, sw, &SearchWorker::findInFiles, Qt::UniqueConnection);
    connect(sw, &SearchWorker::update, mMain->searchDialog(), &SearchDialog::intermediateUpdate, Qt::UniqueConnection);
    connect(sw, &SearchWorker::resultReady, mMain->searchDialog(), &SearchDialog::finalUpdate, Qt::UniqueConnection);

    mThread.start();
    mThread.setPriority(QThread::LowPriority); // search is a background task
}

void Search::stop()
{
    mThread.requestInterruption();
}

void Search::reset()
{
    mFiles.clear();
    mRegex.setPattern("");

    mOptions = QFlags<QTextDocument::FindFlag>();
    mCacheAvailable = false;
    mThread.isInterruptionRequested();
}

void Search::findInDoc(FileMeta* fm)
{
    QTextCursor lastItem = QTextCursor(fm->document());
    QTextCursor item;
    do {
        item = fm->document()->find(mRegex, lastItem, mOptions);
        if (item != lastItem) lastItem = item;
        else break;

        if (!item.isNull()) {
            mResults.append(Result(item.blockNumber()+1, item.columnNumber() - item.selectedText().length(),
                                  item.selectedText().length(), fm->location(), item.block().text().trimmed()));
        }
        if (mResults.size() > MAX_SEARCH_RESULTS-1) break;
    } while (!item.isNull());
}

void Search::findNext(Direction direction)
{
    // create new cache when cached search does not contain results for current file
    bool requestNewCache = !mCacheAvailable
            || mResultHash.find(mMain->fileRepo()->fileMeta(mMain->recent()->editor()
                                                            )->location()) == mResultHash.end();

    if (requestNewCache) {
        mMain->searchDialog()->updateUi(true);
        start();
    }
    selectNextMatch(direction);
}

///
/// \brief SearchDialog::selectNextMatch steps through words in a document
/// \param direction
///
void Search::selectNextMatch(Direction direction, bool firstLevel)
{
    QTextCursor matchSelection;

    bool backwards = direction == Direction::Backward;
    int matchNr = -1;
    bool found = false;

    if (mCacheAvailable) {
        int lineNr = 0;
        int colNr = 0;
        if (AbstractEdit* e = ViewHelper::toAbstractEdit(mMain->recent()->editor())) {
            lineNr = e->textCursor().blockNumber()+1;
            colNr = e->textCursor().columnNumber();
        } else if (TextView* t = ViewHelper::toTextView(mMain->recent()->editor())) {
            lineNr = t->position().y()+1;
            colNr = t->position().x();
        }

        int iterator = backwards ? -1 : 1;
        int start = backwards ? mResults.size()-1 : 0;
        bool allowJumping = false;

        // skip to next entry if file is opened in solver option edit
        if (ViewHelper::toSolverOptionEdit(mMain->recent()->editor())) {
            int selected = mMain->resultsView() ? mMain->resultsView()->selectedItem() : -1;

            // no rows selected, select new depending on direction
            if (selected == -1) selected = backwards ? mResults.size() : 0;

            int newIndex = selected + iterator;
            if (newIndex < 0)
                newIndex = mResults.size()-1;
            else if (newIndex > mResults.size()-1)
                newIndex = 0;

            matchNr = newIndex;

        } else if (mMain->recent()->editor()){ // finds next result from cache by cursor position
            QString file = ViewHelper::location(mMain->recent()->editor());
            for (int i = start; i >= 0 && i < mResults.size(); i += iterator) {
                matchNr = i;
                Result r = mResults.at(i);

                // check if is in same line but behind the cursor
                if (file == r.filepath()) {
                    allowJumping = true;
                    if (backwards) {
                        if (lineNr > r.lineNr() || (lineNr == r.lineNr() && colNr > r.colNr() + r.length())) {
                            found = true;
                            break;
                        }
                    } else {
                        if (lineNr < r.lineNr() || (lineNr == r.lineNr() && colNr <= r.colNr())) {
                            found = true;
                            break;
                        }
                    }
                } else if (file != r.filepath() && allowJumping) {
                    // first match in next file
                    found = true;
                    break;
                }
            }
        }
    }

    // navigation outside of cache
    if (!found) {
        mOutsideOfList = mResults.size() >= MAX_SEARCH_RESULTS;

        int x = 0;
        int y = 0;
        // start over
        if (AbstractEdit* e = ViewHelper::toAbstractEdit(mMain->recent()->editor())) {
            QTextCursor tc = e->textCursor();
            QTextCursor ntc= e->document()->find(mRegex, backwards ? tc.position()-tc.selectedText().length() : tc.position(), mOptions);
            found = !ntc.isNull();
            e->setTextCursor(ntc);
            x = e->textCursor().positionInBlock();
            y = e->textCursor().blockNumber()+1;
        } else if (TextView* t = ViewHelper::toTextView(mMain->recent()->editor())) {
            mSplitSearchContinue = false; // make sure to start a new search
            found = t->findText(mRegex, mOptions, mSplitSearchContinue);
            x = t->position().x();
            y = t->position().y()+1;
        }
        if (mCacheAvailable) emit updateLabelByCursorPos(y, x);

        if (found) return; // exit early, all done

        // still no results found, start over, jump to start/end of file
        if (mOutsideOfList) {
            if (backwards) {
                if (AbstractEdit* e = ViewHelper::toAbstractEdit(mMain->recent()->editor())) {
                    QTextCursor tc = e->textCursor();
                    tc.movePosition(QTextCursor::End);
                    e->setTextCursor(tc);
                } else if (TextView* t = ViewHelper::toTextView(mMain->recent()->editor())) {
                    t->jumpTo(t->knownLines()-1, 0, 0, true);
                }
                matchNr = mResults.size()-1;
                if (firstLevel) selectNextMatch(direction, false); // try jumping once again
            } else { // forwards
                matchNr = 0;
            }

        } else { // startover in available cache
            if (backwards) matchNr = mResults.size()-1;
            else matchNr = 0;
        }
    }

    // jump using cache
    if (mCacheAvailable && matchNr < mResults.size()) {
        ProjectFileNode *node = mMain->projectRepo()->findFile(mResults.at(matchNr).filepath());
        if (!node) EXCEPT() << "File not found: " << mResults.at(matchNr).filepath();

        node->file()->jumpTo(node->runGroupId(), true, mResults.at(matchNr).lineNr()-1,
                             qMax(mResults.at(matchNr).colNr(), 0), mResults.at(matchNr).length());

        updateLabelByCursorPos(-1, -1);
    }

    // update results view
    if (mMain->resultsView() && !mMain->resultsView()->isOutdated())
        mMain->resultsView()->selectItem(matchNr);
}

///
/// \brief SearchDialog::replaceUnopened replaces in files where there is currently no editor open
/// \param fm file
/// \param regex find
/// \param replaceTerm replace with
///
// TODO(RG): check the performance of this for large files
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
int Search::replaceOpened(FileMeta* fm, QRegularExpression regex, QString replaceTerm, QFlags<QTextDocument::FindFlag> flags)
{
    QTextCursor item;
    QTextCursor lastItem;
    int hits = 0;

    QTextCursor tc;
    if (fm->editors().size() > 0)
        tc = ViewHelper::toAbstractEdit(fm->editors().first())->textCursor();

    tc.beginEditBlock();
    do {
        item = fm->document()->find(regex, lastItem, flags);
        lastItem = item;

        if (!item.isNull()) {
            item.insertText(replaceTerm);
            hits++;
        }
    } while(!item.isNull());
    tc.endEditBlock();

    return hits;
}

void Search::finished()
{
    mSearching = false;

    for (Result r : mResults)
        mResultHash[r.filepath()].append(r);

    if (AbstractEdit* ae = ViewHelper::toAbstractEdit(mMain->recent()->editor()))
        ae->updateExtraSelections();
    else if (TextView* tv = ViewHelper::toTextView(mMain->recent()->editor()))
        tv->updateExtraSelections();

    mCacheAvailable = true;
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

void Search::replaceNext(QRegularExpression regex, QString replacementText)
{
    AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
    if (!edit || edit->isReadOnly()) return;

    QRegularExpressionMatch match = regex.match(edit->textCursor().selectedText());

    if (edit->textCursor().hasSelection() && match.hasMatch() &&
            match.captured(0) == edit->textCursor().selectedText()) {
        edit->textCursor().insertText(replacementText);
    }

    selectNextMatch();
}

void Search::replaceAll(QList<FileMeta*> files, QRegularExpression regex, QString replacementText)
{
    if (regex.pattern().isEmpty()) return;

    QList<FileMeta*> opened;
    QList<FileMeta*> unopened;

    int matchedFiles = 0;

    // sort and filter FMs by editability and modification state
    for (FileMeta* fm : files) {
        if (fm->isReadOnly()) {
            files.removeOne(fm);
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
    QString searchTerm = regex.pattern();
    QString replaceTerm = replacementText;
    QMessageBox msgBox;
    if (files.length() == 0) {
        msgBox.setText("No files matching criteria.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;

    } else if (matchedFiles == 1) {
        msgBox.setText("Are you sure you want to replace all occurrences of '" +
                       searchTerm + "' with '" + replaceTerm + "' in file "
                       + files.first()->name() + ". This action cannot be undone. Are you sure?");
    } else if (matchedFiles >= 2) {
        msgBox.setText("Are you sure you want to replace all occurrences of '" +
                       searchTerm + "' with '" + replaceTerm + "' in " + QString::number(matchedFiles) + " files. " +
                       "This action cannot be undone. Are you sure?");
        QString detailedText;
        msgBox.setInformativeText("Click \"Show Details...\" to show selected files.");

        for (FileMeta* fm : files)
            detailedText.append(fm->location()+"\n");
        detailedText.append("\nThese files do not necessarily have any matches in them. "
                            "This is just a representation of the selected scope in the search window. "
                            "Press \"Search\" to see actual matches that will be replaced.");
        msgBox.setDetailedText(detailedText);
    }
    QPushButton *ok = msgBox.addButton(QMessageBox::Ok);
    QPushButton *cancel = msgBox.addButton(QMessageBox::Cancel);
    QPushButton *search = msgBox.addButton("Search", QMessageBox::RejectRole);
    msgBox.setDefaultButton(search);

    int hits = 0;
    msgBox.exec();
    if (msgBox.clickedButton() == ok) {

        mMain->searchDialog()->setSearchStatus(Search::Replacing);
        QApplication::processEvents(QEventLoop::AllEvents, 10); // to show change in UI

        for (FileMeta* fm : opened)
            hits += replaceOpened(fm, regex, replaceTerm, mOptions);

        for (FileMeta* fm : unopened)
            hits += replaceUnopened(fm, regex, replaceTerm);

        mMain->searchDialog()->setSearchStatus(Search::Clear);
    } else if (msgBox.clickedButton() == search) {
        start();
        return;
    } else if (msgBox.clickedButton() == cancel) {
        return;
    }

    QMessageBox ansBox;
    ansBox.setText(QString::number(hits) + " occurrences of '" + searchTerm + "' were replaced with '" + replaceTerm + "'.");
    ansBox.addButton(QMessageBox::Ok);
    ansBox.exec();
}

}
}
}
