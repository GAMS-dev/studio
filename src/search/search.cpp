/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "editors/sysloglocator.h"
#include "editors/textview.h"
#include "fileworker.h"
#include "viewhelper.h"
#include "file/textfilesaver.h"

#include "encoding.h"

namespace gams {
namespace studio {
namespace search {

using Qt::UniqueConnection;

Search::Search(SearchDialog *sd, AbstractSearchFileHandler* fileHandler)
    : mSearchDialog(sd)
    , mFileHandler(fileHandler)
{
    connect(this, &Search::invalidateResults, mSearchDialog, &SearchDialog::invalidateResultsView);
    connect(this, &Search::selectResult, mSearchDialog, &SearchDialog::selectResult);
}

Search::~Search()
{
    mFileThread.requestInterruption();
    mFileThread.deleteLater();

    mSearchThread.requestInterruption();
    mSearchThread.deleteLater();
}

void Search::start(const SearchParameters &parameters)
{
    if (mSearching) return;

    mLastSearchParameters = parameters;
    mStopRequested = false;

    resetResults();

    // check for early abort
    if (parameters.regex.pattern().isEmpty() ||
        (mSearchDialog->selectedScope() == Scope::Folder && parameters.path.isEmpty())) {
        mSearchDialog->setSearchStatus(Search::Clear);
        mSearchDialog->finalUpdate();
        return;
    } else {
        mSearching = true;
        mSearchDialog->setSearchStatus(Search::CollectingFiles);
        mSearchDialog->updateDialogState();
    }

    // selection scope special treatment
    if (mSearchDialog->selectedScope() == Scope::Selection) {
        findInSelection(parameters.showResults);
        return;
    } else if (mSearchDialog->selectedScope() != Scope::Folder) {
        runSearch(mSearchDialog->getFilesByScope(parameters));
    } else {
        // async file collection
        FileWorker* fw = new FileWorker(parameters, mFileHandler);
        fw->moveToThread(&mFileThread);

        connect(&mFileThread, &QThread::started, fw, &FileWorker::collectFilesInFolder);
        connect(fw, &FileWorker::filesCollected, this, &Search::runSearch);
        connect(fw, &FileWorker::filesCollected, &mFileThread, &QThread::quit);
        connect(&mFileThread, &QThread::finished, fw, &QObject::deleteLater);

        mFileThread.start();
        mFileThread.setPriority(QThread::LowPriority); // search is a background task
    }
}

void Search::runSearch(const QList<SearchFile> &files)
{
    if (mStopRequested) {
        mStopRequested = false;
        mSearching = false;
        mSearchDialog->finalUpdate();
        return;
    }

    mSearchDialog->setSearchedFiles(files.count());
    mSearchDialog->setSearchStatus(Search::Searching);

    QList<SearchFile> unmodified;
    QList<SearchFile> modified; // need to be treated differently

    FileMeta* currentFile = mFileHandler->fileMeta(mSearchDialog->currentEditor());
    for (const SearchFile& sf : std::as_const(files)) {
        if (FileMeta* fm = sf.fileMeta()) {
            // skip certain file types
            if (fm->kind() == FileKind::Gdx || fm->kind() == FileKind::Ref)
                continue;
            // sort files by modified, current file first
            if (fm->isModified() && fm->document()) {
                if (fm == currentFile) modified.insert(0, fm);
                else modified << fm;
            } else {
                if (fm == currentFile) unmodified.insert(0, sf);
            }
        } else {
            unmodified << sf;
        }
    }

    // process modified files before the parallel part starts
    for (const SearchFile &sf : std::as_const(modified))
        findInDoc(sf.fileMeta());

    // start background task first
    SearchWorker* sw = new SearchWorker(unmodified, mLastSearchParameters.regex, &mResults,
                                        mLastSearchParameters.showResults);
    sw->moveToThread(&mSearchThread);

    connect(&mSearchThread, &QThread::finished, this, &Search::finished, UniqueConnection);
    connect(&mSearchThread, &QThread::finished, sw, &QObject::deleteLater, UniqueConnection);
    connect(&mSearchThread, &QThread::started, sw, &SearchWorker::findInFiles, UniqueConnection);
    connect(sw, &SearchWorker::update, mSearchDialog, &SearchDialog::intermediateUpdate, UniqueConnection);
    connect(sw, &SearchWorker::showResults, mSearchDialog, &SearchDialog::relaySearchResults, UniqueConnection);

    mSearchThread.start();
    mSearchThread.setPriority(QThread::LowPriority); // search is a background task
}

void Search::requestStop()
{
    mStopRequested = true;
    mFileThread.requestInterruption();
    mSearchThread.requestInterruption();
}

void Search::activeFileChanged()
{
    FileMeta* current = mSearchDialog->fileHandler()->fileMeta(mSearchDialog->currentEditor());
    if (current && !mResultHash.contains(current->location()))
        mCacheAvailable = false;
}

void Search::reset()
{
    resetResults();

    invalidateCache();
    mOutsideOfList = false;
    mLastMatchInOpt = -1;
    mSearchDialog->setSearchStatus(Status::Clear);

    mLastSearchParameters = SearchParameters();

    mSearchDialog->setSearchedFiles(0);

    mFileThread.requestInterruption();
    mSearchThread.requestInterruption();

    emit invalidateResults();
}

void Search::resetResults()
{
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
        tv->findInSelection(mLastSearchParameters.regex,
                            mSearchDialog->fileHandler()->fileMeta(mSearchSelectionFile),
                            &mResults, showResults);
    }
    mSearchDialog->updateClearButton();

    // nothing more to do, update UI and return
    finished();
}

void Search::checkFileChanged(const FileId &fileId)
{
    if (mSearchSelectionFile != fileId) {
        emit invalidateResults();
        mSearchSelectionFile = fileId;
    }
}

void Search::findInDoc(FileMeta* fm)
{
    QTextCursor cursor;
    QTextCursor lastCursor = QTextCursor(fm->document());

    do {
        cursor = fm->document()->find(mLastSearchParameters.regex, lastCursor,
                                    createFindFlags(mLastSearchParameters));
        if (cursor != lastCursor) lastCursor = cursor;
        else break;

        if (!cursor.isNull()) {
            mResults.append(Result(cursor.blockNumber()+1,
                                   cursor.positionInBlock() - cursor.selectedText().length(),
                                   cursor.selectedText().length(),
                                   fm->location(),
                                   fm->projectId(),
                                   cursor.block().text()));
        }
        if (mResults.size() > MAX_SEARCH_RESULTS) break;
    } while (!cursor.isNull());
}

void Search::findNext(Direction direction)
{
    if (!mCacheAvailable) {
        emit invalidateResults();
        emit updateUI();
        // generate new cache
        start(mSearchDialog->createSearchParameters(false, false, direction == Search::Backward));
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
            if (file == r.filePath()) {
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
            } else if (file != r.filePath() && allowJumping) {
                // first match in next file
                return i;
            }
        }
    }
    return -1; // not found
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

QFlags<QTextDocument::FindFlag> Search::createFindFlags(const SearchParameters &parameters, Direction direction) {
    QFlags<QTextDocument::FindFlag> searchOptions;
    searchOptions.setFlag(QTextDocument::FindBackward, direction == Direction::Backward);
    searchOptions.setFlag(QTextDocument::FindCaseSensitively, parameters.caseSensitive);
    return searchOptions;
}

int Search::NavigateOutsideCache(Direction direction, bool firstLevel)
{
    int matchNr = -1;
    bool found = false;

    QFlags<QTextDocument::FindFlag> options = createFindFlags(mLastSearchParameters, direction);

    // do not move cursor in document that is not in scope:
    if (mSearchDialog->getFilesByScope(mLastSearchParameters).contains(mSearchDialog->fileHandler()->fileMeta(mSearchDialog->currentEditor()))) {
        if (AbstractEdit* e = ViewHelper::toAbstractEdit(mSearchDialog->currentEditor())) {

            QTextCursor tc = e->textCursor();
            if (!firstLevel) {
                if (direction == Direction::Backward) tc.movePosition(QTextCursor::End);
                else tc.movePosition(QTextCursor::Start);
            }

            QTextCursor ntc = e->document()->find(mLastSearchParameters.regex, direction == Direction::Backward
                                                  ? tc.position()-tc.selectedText().length()
                                                  : tc.position(), options);
            found = !ntc.isNull();
            if (found) {
                e->jumpTo(ntc.blockNumber()+1, ntc.columnNumber());
                e->setTextCursor(ntc);
            }

        } else if (TextView* t = ViewHelper::toTextView(mSearchDialog->currentEditor())) {
            mSplitSearchContinue = !firstLevel;
            found = t->findText(mLastSearchParameters.regex, options, mSplitSearchContinue);
        }
    }

    // try again
    if (!found && !mSearching) {
        matchNr = findNextEntryInCache(direction);
        found = matchNr != -1;

        mSearchDialog->jumpToResult(matchNr);
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
    mSearchDialog->jumpToResult(matchNr);

    return matchNr;
}

bool Search::hasResultsForFile(const QString &filePath)
{
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
int Search::replaceUnopened(FileMeta* fm, const SearchParameters &parameters)
{
    QFile file(fm->location());
    QStringDecoder decoder = Encoding::createDecoder(fm->encoding());
    int hits = 0;

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        SysLogLocator::systemLog()->append("Unable to open file " + fm->location(), LogMsgType::Error);
        return 0;
    }

    QString content;
    while (!file.atEnd()) {
        QByteArray arry = file.readLine();
        // TODO(JM) when switching back to QTextStream this can be removed, as stream doesn't append the \n
        if (arry.endsWith('\n')) {
            if (arry.length() > 1 && arry.at(arry.length()-2) == '\r')
                arry.chop(2);
            else
                arry.chop(1);
        }

        QString line = decoder.decode(arry);

        if (parameters.regex.captureCount() > 0) {
            QRegularExpressionMatchIterator matchIter = parameters.regex.globalMatch(line);

            // iterate over matches in this line
            while(matchIter.hasNext()) {
                QRegularExpressionMatch match = matchIter.next();
                QString modifiedReplaceTerm = parameters.replaceTerm;

                // replace capture groups placeholders with content
                for(int i = 1; i <= parameters.regex.captureCount(); i++) {
                    QRegularExpression replaceGroup("\\$" + QString::number(i));
                    QString captured = match.capturedTexts().at(i);
                    modifiedReplaceTerm.replace(replaceGroup, captured);
                }

                // replace match with filled replace term
                line.replace(match.capturedStart(), match.capturedLength(), modifiedReplaceTerm);
                hits++;
            }
        } else {
            hits += line.count(parameters.regex);
            line.replace(parameters.regex, parameters.replaceTerm);
        }
        content += line + "\n";
    }
    file.close();
    if (hits) {
        // write temp-file
        TextFileSaver outFile;
        if (!outFile.open(fm->location()))
            return 0;
        QStringEncoder encoder = Encoding::createEncoder(fm->encoding());
        outFile.write(encoder.encode(content));
        outFile.close();
    }
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
int Search::replaceOpened(FileMeta* fm, const SearchParameters &parameters)
{
    AbstractEdit* ae = ViewHelper::toAbstractEdit(fm->editors().constFirst());

    int hits = 0;
    if (ae && fm->editors().size() > 0) {
        hits = ae->replaceAll(fm, parameters.regex, parameters.replaceTerm,
                              createFindFlags(parameters), mSearchDialog->selectedScope() == Scope::Selection);
        mCacheAvailable = false;
    }
    return hits;
}

void Search::finished()
{
    mSearching = false;

    for (const Result &r : std::as_const(mResults))
        mResultHash[r.filePath()].append(r);

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

Scope Search::scope() const
{
    return mLastSearchParameters.scope;
}

bool Search::caseSensitive() const
{
    return mLastSearchParameters.caseSensitive;
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
    return mLastSearchParameters.regex;
}

bool Search::isSearching() const
{
    return mSearching;
}

QList<Result> Search::results() const
{
    return mResults;
}

QList<Result> Search::filteredResultList(const QString &fileLocation)
{
    return mResultHash[fileLocation];
}

void Search::replaceNext(const QString& replacementText)
{
    AbstractEdit* edit = ViewHelper::toAbstractEdit(mSearchDialog->currentEditor());
    if (!edit) return;

    edit->replaceNext(mLastSearchParameters.regex, replacementText, mSearchDialog->selectedScope() == Scope::Selection);

    start(mSearchDialog->createSearchParameters(false, true)); // refresh cache
    selectNextMatch();
}

void Search::replaceAll(SearchParameters parameters)
{
    if (parameters.regex.pattern().isEmpty()) return;

    QList<FileMeta*> opened;
    QList<FileMeta*> unopened;

    // sort and filter FMs by editability and modification state
    QList<SearchFile> files = mSearchDialog->getFilesByScope(parameters);
    mLastSearchParameters = parameters;

    int matchedFiles = 0;

    for (const SearchFile &file : std::as_const(files)) {
        FileMeta* fm = file.fileMeta();

        if (!fm)
            fm = mFileHandler->findFile(file.path());

        if (fm && fm->document()) {
            if (!opened.contains(fm)) opened << fm;
        } else {
            if (!unopened.contains(fm)) unopened << fm;
        }

        matchedFiles++;
    }

    QString fileName = !files.count()
                           ? "" : files.first().fileMeta() ? files.first().fileMeta()->name()
                                                 : QFileInfo(files.first().path()).fileName();

    // user interaction
    QMessageBox msgBox;
    if (files.size() == 0) {
        msgBox.setText("No files matching criteria.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;

    } else if (matchedFiles == 1 && mSearchDialog->selectedScope() != Scope::Selection) {
        msgBox.setText("Are you sure you want to replace all occurrences of '" + parameters.searchTerm
                       + "' with '" + parameters.replaceTerm + "' in file " + fileName + "?");
    } else if (mSearchDialog->selectedScope() == Scope::Selection) {
        msgBox.setText("Are you sure you want to replace all occurrences of '" + parameters.searchTerm
                       + "' with '" + parameters.replaceTerm + "' in the selected text in file "
                       + fileName + "?");
    } else {
        msgBox.setText("Are you sure you want to replace all occurrences of '" +
                       parameters.searchTerm + "' with '" + parameters.replaceTerm + "' in "
                       + QString::number(matchedFiles) + " files? " +
                       "This action cannot be undone!");
        QString detailedText;
        msgBox.setInformativeText("Click \"Show Details...\" to show selected files.");

        for (const SearchFile &file : std::as_const(files))
            detailedText.append(file.path()+"\n");

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
        for (FileMeta* fm : std::as_const(opened))
            hits += replaceOpened(fm, parameters);
        for (FileMeta* fm : std::as_const(unopened))
            hits += replaceUnopened(fm, parameters);

        mSearchDialog->searchParameterChanged();
    } else if (msgBox.clickedButton() == preview) {
        parameters.showResults = true;
        start(parameters);
        return;
    } else if (msgBox.clickedButton() == cancel) {
        return;
    }

    QMessageBox ansBox;
    if (hits == 1)
        ansBox.setText("1 occurrences of '" + parameters.searchTerm + "' was replaced with '"
                       + parameters.replaceTerm + "'.");
    else
        ansBox.setText(QString::number(hits) + " occurrences of '" + parameters.searchTerm
                        + "' were replaced with '" + parameters.replaceTerm + "'.");
    ansBox.addButton(QMessageBox::Ok);
    ansBox.exec();
}

} // namespace search
}
}
