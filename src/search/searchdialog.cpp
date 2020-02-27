/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "settings.h"
#include "syntax.h"
#include "file.h"
#include "exception.h"
#include "searchresultlist.h"
#include "searchworker.h"
#include "option/solveroptionwidget.h"
#include "settingslocator.h"
#include "editors/viewhelper.h"
#include "lxiviewer/lxiviewer.h"
#include "../keys.h"

#include <QMessageBox>
#include <QTextDocumentFragment>
#include <QThread>



namespace gams {
namespace studio {
namespace search {

SearchDialog::SearchDialog(MainWindow *parent) :
    QDialog(parent), ui(new Ui::SearchDialog), mMain(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    Settings *mSettings = SettingsLocator::settings();

    ui->setupUi(this);
    ui->cb_regex->setChecked(mSettings->searchUseRegex());
    ui->cb_caseSens->setChecked(mSettings->searchCaseSens());
    ui->cb_wholeWords->setChecked(mSettings->searchWholeWords());
    ui->combo_scope->setCurrentIndex(mSettings->selectedScopeIndex());
    ui->lbl_nrResults->setText("");
    ui->combo_search->setAutoCompletion(false);
    adjustSize();
}

SearchDialog::~SearchDialog()
{
    delete ui;
    delete mCachedResults;
}

void SearchDialog::on_btn_Replace_clicked()
{
    AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
    if (!edit || edit->isReadOnly()) return;

    insertHistory();
    mIsReplacing = true;
    QRegularExpression regex = createRegex();
    QRegularExpressionMatch match = regex.match(edit->textCursor().selectedText());

    if (edit->textCursor().hasSelection() && match.hasMatch() &&
            match.capturedLength() == edit->textCursor().selectedText().length()) {
        edit->textCursor().insertText(ui->txt_replace->text());
    }

    findNext(SearchDialog::Forward, true);
    mIsReplacing = false;
}

void SearchDialog::on_btn_ReplaceAll_clicked()
{
    insertHistory();
    replaceAll();
}

void SearchDialog::on_btn_FindAll_clicked()
{
    if (!mSearching) {
        if (ui->combo_search->currentText().isEmpty()) return;
        mHasChanged = false;

        setSearchOngoing(true);
        clearResults();
        insertHistory();

        mShowResults = true;
        mCachedResults = new SearchResultList(createRegex());
        findInFiles(mCachedResults, getFilesByScope());
    } else {
        setSearchOngoing(false);
        mThread.requestInterruption();
    }
}

void SearchDialog::intermediateUpdate()
{
    setSearchStatus(SearchStatus::Searching);
}

void SearchDialog::finalUpdate()
{
    setSearchOngoing(false);

    if (mShowResults) {
        mMain->showResults(mCachedResults);
        resultsView()->resizeColumnsToContent();
    } else {
        AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
        TextView* tv = ViewHelper::toTextView(mMain->recent()->editor());

        if (edit || tv) selectNextMatch(SearchDirection::Forward);
    }

    updateEditHighlighting();

    if (mCachedResults && !mCachedResults->size()) setSearchStatus(SearchStatus::NoResults);
    else updateFindNextLabel(0, 0);
}

void SearchDialog::setSearchOngoing(bool searching)
{
    mSearching = searching;

    if (searching)
        ui->btn_FindAll->setText("Abort");
    else
        ui->btn_FindAll->setText("Find All");

    // deactivate actions while search is ongoing
    ui->btn_forward->setEnabled(!searching);
    ui->btn_back->setEnabled(!searching);
    ui->btn_Replace->setEnabled(!searching);
    ui->btn_ReplaceAll->setEnabled(!searching);
    ui->btn_clear->setEnabled(!searching);
    ui->cb_regex->setEnabled(!searching);
    ui->cb_caseSens->setEnabled(!searching);
    ui->cb_wholeWords->setEnabled(!searching);
    ui->combo_filePattern->setEnabled(!searching);
    ui->combo_scope->setEnabled(!searching);
    ui->combo_search->setEnabled(!searching);
    ui->txt_replace->setEnabled(!searching);
    ui->label->setEnabled(!searching);
    ui->label_2->setEnabled(!searching);
    ui->label_3->setEnabled(!searching);
}

///
/// \brief SearchDialog::findInFiles starts a search in files. distinguishes between (non-)modified files.
/// \param mMutex for parallelisation
/// \param fml list of files to search
/// \param skipFilters enable for result caching (find next/prev)
///
void SearchDialog::findInFiles(SearchResultList* collection, QList<FileMeta*> fml)
{
    QList<FileMeta*> unmodified;
    QList<FileMeta*> modified; // need to be treated differently

    for(FileMeta* fm : fml) {

        // skip certain file types
        if (fm->kind() == FileKind::Gdx || fm->kind() == FileKind::Lxi || fm->kind() == FileKind::Ref)
            continue;

        // sort files by modified
        if (fm->isModified()) modified << fm;
        else unmodified << fm;
    }

    // non-parallel first
    for (FileMeta* fm : modified)
        findInDoc(createRegex(), fm, collection);

    SearchWorker* sw = new SearchWorker(mMutex, createRegex(), unmodified, collection);
    sw->moveToThread(&mThread);

    connect(&mThread, &QThread::finished, sw, &QObject::deleteLater, Qt::UniqueConnection);
    connect(this, &SearchDialog::startSearch, sw, &SearchWorker::findInFiles, Qt::UniqueConnection);
    connect(sw, &SearchWorker::update, this, &SearchDialog::intermediateUpdate, Qt::UniqueConnection);
    connect(sw, &SearchWorker::resultReady, this, &SearchDialog::finalUpdate, Qt::UniqueConnection);

    mThread.start();
    emit startSearch();
}

void SearchDialog::findInDoc(QRegularExpression searchRegex, FileMeta* fm, SearchResultList* collection)
{
    QTextCursor lastItem = QTextCursor(fm->document());
    QTextCursor item;
    QFlags<QTextDocument::FindFlag> flags = setFlags(SearchDirection::Forward);
    do {
        item = fm->document()->find(searchRegex, lastItem, flags);
        if (item != lastItem) lastItem = item;
        else break;

        if (!item.isNull()) {
            collection->addResult(item.blockNumber()+1, item.columnNumber() - item.selectedText().length(),
                                  item.selectedText().length(), fm->location(), item.block().text().trimmed());
        }
        if (collection->size() > MAX_SEARCH_RESULTS-1) break;
    } while (!item.isNull());
}

QList<FileMeta*> SearchDialog::getFilesByScope(bool ignoreReadOnly)
{
    QList<FileMeta*> files;
    QRegExp fileFilter(ui->combo_filePattern->currentText().trimmed());
    fileFilter.setPatternSyntax(QRegExp::Wildcard);

    switch (ui->combo_scope->currentIndex()) {
    case SearchScope::ThisFile:
        if (mMain->recent()->editor())
            files.append(mMain->fileRepo()->fileMeta(mMain->recent()->editor()));
        break;
    case SearchScope::ThisGroup:
    {
        ProjectFileNode* p = mMain->projectRepo()->findFileNode(mMain->recent()->editor());
        if (!p) return files;
        for (ProjectFileNode *c :p->parentNode()->listFiles(true)) {
            if (!files.contains(c->file()))
                files.append(c->file());
        }
    }
        break;
    case SearchScope::OpenTabs:
        files = QList<FileMeta*>::fromVector(mMain->fileRepo()->openFiles());
        break;
    case SearchScope::AllFiles:
        files = mMain->fileRepo()->fileMetas();
        break;
    default:
        break;
    }

    // apply filter
    QList<FileMeta*> res;
    for (FileMeta* fm : files) {
        if ((ui->combo_scope->currentIndex() == SearchScope::ThisFile || fileFilter.indexIn(fm->location()) != -1) && (!ignoreReadOnly || !fm->isReadOnly()))
            res.append(fm);
    }
    return res;
}

void SearchDialog::replaceAll()
{
    if (ui->combo_search->currentText().isEmpty()) return;

    QList<FileMeta*> fml = getFilesByScope(true);

    QList<FileMeta*> opened;
    QList<FileMeta*> unopened;

    int matchedFiles = 0;

    mHasChanged = false;

    // sort and filter FMs by editability and modification state
    for (FileMeta* fm : fml) {
        if (fm->isReadOnly()) {
            fml.removeOne(fm);
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
    QString searchTerm = ui->combo_search->currentText();
    QString replaceTerm = ui->txt_replace->text();
    QMessageBox msgBox;
    if (fml.length() == 0) {
        msgBox.setText("No files matching criteria.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;

    } else if (matchedFiles == 1) {
        msgBox.setText("Are you sure you want to replace all occurrences of '" +
                       searchTerm + "' with '" + replaceTerm + "' in file "
                       + fml.first()->name() + ". This action cannot be undone. Are you sure?");
    } else if (matchedFiles >= 2) {
        msgBox.setText("Are you sure you want to replace all occurrences of '" +
                       searchTerm + "' with '" + replaceTerm + "' in " + QString::number(matchedFiles) + " files. " +
                       "This action cannot be undone. Are you sure?");
        QString detailedText;
        msgBox.setInformativeText("Click \"Show Details...\" to show selected files.");

        for (FileMeta* fm : fml)
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

        setSearchStatus(SearchStatus::Replacing);
        QApplication::processEvents(QEventLoop::AllEvents, 10); // to show change in UI

        QRegularExpression regex = createRegex();
        QFlags<QTextDocument::FindFlag> flags = setFlags(SearchDirection::Forward);

        for (FileMeta* fm : opened) {
            hits += replaceOpened(fm, regex, replaceTerm, flags);
        }

        for (FileMeta* fm : unopened) {
            hits += replaceUnopened(fm, regex, replaceTerm);
        }

        setSearchStatus(SearchStatus::Clear);
    } else if (msgBox.clickedButton() == search) {
        mShowResults = true;
        invalidateCache();
        mCachedResults = new SearchResultList(createRegex());
        findInFiles(mCachedResults, fml);
        return;
    } else if (msgBox.clickedButton() == cancel) {
        return;
    }

    QMessageBox ansBox;
    ansBox.setText(QString::number(hits) + " occurrences of '" + searchTerm + "' were replaced with '" + replaceTerm + "'.");
    ansBox.addButton(QMessageBox::Ok);
    ansBox.exec();

    invalidateCache();
}

///
/// \brief SearchDialog::replaceUnopened replaces in files where there is currently no editor open
/// \param fm file
/// \param regex find
/// \param replaceTerm replace with
///
int SearchDialog::replaceUnopened(FileMeta* fm, QRegularExpression regex, QString replaceTerm)
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
int SearchDialog::replaceOpened(FileMeta* fm, QRegularExpression regex, QString replaceTerm, QFlags<QTextDocument::FindFlag> flags)
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

void SearchDialog::updateSearchCache(bool ignoreReadOnly)
{
    QApplication::sendPostedEvents();

    if (mCachedResults) delete mCachedResults;
    mCachedResults = new SearchResultList(createRegex());

    mShowResults = false;
    findInFiles(mCachedResults, getFilesByScope(ignoreReadOnly));
    mHasChanged = false;
}

void SearchDialog::findNext(SearchDirection direction, bool ignoreReadOnly)
{
    if (ui->combo_search->currentText() == "") return;

    // create new cache when cached search does not contain results for current file. user probably changed tab and a new search needs to start
    bool requestNewCache = mCachedResults &&
            mCachedResults->filteredResultList(mMain->fileRepo()->fileMeta(mMain->recent()->editor())->location()).size() == 0;

    if (!mCachedResults || mHasChanged || requestNewCache) {
        setSearchOngoing(true);
        invalidateCache();
        updateSearchCache(ignoreReadOnly);
        QApplication::processEvents(QEventLoop::AllEvents, 50);
    }
    if (!mIsReplacing) selectNextMatch(direction);
}

///
/// \brief SearchDialog::selectNextMatch steps through words in a document
/// \param direction
/// \param second is second time entering this function, to avoid too deep recursion
///
void SearchDialog::selectNextMatch(SearchDirection direction)
{
    QTextCursor matchSelection;
    QRegularExpression searchRegex = createRegex();

    setSearchStatus(SearchStatus::Searching);

    int lineNr = 0;
    int colNr = 0;
    bool backwards = direction == SearchDirection::Backward;

    if (AbstractEdit* e = ViewHelper::toAbstractEdit(mMain->recent()->editor())) {
        lineNr = e->textCursor().blockNumber()+1;
        colNr = e->textCursor().columnNumber();
    } else if (TextView* t = ViewHelper::toTextView(mMain->recent()->editor())) {
        lineNr = t->position().y()+1;
        colNr = t->position().x();
    }

    QList<Result> resultList = mCachedResults->resultsAsList();
    if (resultList.size() == 0) {
        setSearchStatus(SearchStatus::NoResults);
        return;
    }

    const Result* res = nullptr;
    int iterator = backwards ? -1 : 1;
    int start = backwards ? resultList.size()-1 : 0;
    bool allowJumping = false;
    int matchNr = -1;

    // skip to next entry if file is opened in solver option edit
    if (ViewHelper::toSolverOptionEdit(mMain->recent()->editor())) {
        int selected = resultsView() ? resultsView()->selectedItem() : -1;

        // no rows selected, select new depending on direction
        if (selected == -1) selected = backwards ? resultList.size() : 0;

        int newIndex = selected + iterator;
        if (newIndex < 0)
            newIndex = resultList.size()-1;
        else if (newIndex > resultList.size()-1)
            newIndex = 0;

        res = &resultList.at(newIndex);
        matchNr = newIndex;

    } else if (mMain->recent()->editor()){
        QString file = ViewHelper::location(mMain->recent()->editor());
        for (int i = start; i >= 0 && i < resultList.size(); i += iterator) {
            matchNr = i;
            Result r = resultList.at(i);

            // check if is in same line but behind the cursor
            if (file == r.filepath()) {
                allowJumping = true;
                if (backwards) {
                    if (lineNr > r.lineNr() || (lineNr == r.lineNr() && colNr > r.colNr() + r.length())) {
                        res = &r;
                        break;
                    }
                } else {
                    if (lineNr < r.lineNr() || (lineNr == r.lineNr() && colNr <= r.colNr())) {
                        res = &r;
                        break;
                    }
                }
            } else if (file != r.filepath() && allowJumping) {
                // first match in next file
                res = &r;
                break;
            }
        }
    }

    if (!res) {
        if (backwards) res = &resultList.last();
        else res = &resultList.first();

        matchNr = resultList.indexOf(*res);
    }

    // jump to
    ProjectFileNode *node = mMain->projectRepo()->findFile(res->filepath());
    if (!node) EXCEPT() << "File not found: " << res->filepath();
    node->file()->jumpTo(node->runGroupId(), true, res->lineNr()-1, qMax(res->colNr(), 0), res->length());

    // update ui
    if (resultsView() && !resultsView()->isOutdated()) resultsView()->selectItem(matchNr);
    updateNrMatches(matchNr+1);
}

void SearchDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    autofillSearchField();
    updateReplaceActionAvailability();
}

void SearchDialog::on_searchNext()
{
    findNext(SearchDialog::Forward);
}

void SearchDialog::on_searchPrev()
{
    findNext(SearchDialog::Backward);
}

void SearchDialog::on_documentContentChanged(int from, int charsRemoved, int charsAdded)
{
    Q_UNUSED(from)  Q_UNUSED(charsRemoved)  Q_UNUSED(charsAdded)
    searchParameterChanged();
}

void SearchDialog::keyPressEvent(QKeyEvent* e)
{
    if ( isVisible() && ((e->key() == Qt::Key_Escape) || (e == Hotkey::SearchOpen)) ) {
        e->accept();
        mMain->setSearchWidgetPos(pos());
        hide();
        if (mMain->projectRepo()->findFileNode(mMain->recent()->editor())) {
            if (lxiviewer::LxiViewer* lv = ViewHelper::toLxiViewer(mMain->recent()->editor()))
                lv->textView()->setFocus();
            else
                mMain->recent()->editor()->setFocus();
        }

    } else if (e == Hotkey::SearchFindPrev) {
        e->accept();
        on_searchPrev();
    } else if (e == Hotkey::SearchFindNext) {
        e->accept();
        on_searchNext();
    } else if (e->modifiers() & Qt::ShiftModifier && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)) {
        e->accept();
        on_btn_FindAll_clicked();
    }
    QDialog::keyPressEvent(e);
}

void SearchDialog::searchResume()
{
    if (mSplitSearchView && mSplitSearchView == ViewHelper::toTextView(mMain->recent()->editor())) {
        bool found = mSplitSearchView->findText(createRegex(), mSplitSearchFlags, mSplitSearchContinue);
        if (found) updateFindNextLabel(mSplitSearchView->position().y()+1, mSplitSearchView->position().x());

        if (mSplitSearchContinue) {
            setSearchStatus(SearchStatus::Searching);
            QTimer::singleShot(50, this, &SearchDialog::searchResume);
        } else {
            if (!found) setSearchStatus(SearchStatus::NoResults);
            mSplitSearchView = nullptr;
        }
    }
    setSearchOngoing(false);
}

void SearchDialog::on_combo_scope_currentIndexChanged(int)
{
    searchParameterChanged();
    updateReplaceActionAvailability();
}

void SearchDialog::on_btn_back_clicked()
{
    insertHistory();
    findNext(SearchDialog::Backward);
}

void SearchDialog::on_btn_forward_clicked()
{
    insertHistory();
    findNext(SearchDialog::Forward);
}

void SearchDialog::on_btn_clear_clicked()
{
    clearSearch();
}

void SearchDialog::on_cb_wholeWords_stateChanged(int arg1)
{
    Q_UNUSED(arg1)
    searchParameterChanged();
}

void SearchDialog::on_cb_regex_stateChanged(int arg1)
{
    Q_UNUSED(arg1)
    searchParameterChanged();
}

///
/// \brief SearchDialog::updateFindNextLabel calculates match number from cursor position and updates label
/// \param matchSelection cursor position to calculate result number
///
void SearchDialog::updateFindNextLabel(int lineNr, int colNr)
{
    setSearchOngoing(false);

    QString file = "";
    if(mMain->recent()->editor()) file = ViewHelper::location(mMain->recent()->editor());

    // if unknown get cursor from current editor
    if (lineNr == 0 || colNr == 0) {
        AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
        TextView* tv = ViewHelper::toTextView(mMain->recent()->editor());
        QTextCursor tc;
        if (edit) tc = edit->textCursor();
        else if (tv) tc = tv->edit()->textCursor();

        lineNr = tc.blockNumber()+1;
        colNr = tc.columnNumber();
    }

    // find match by cursor position
    QList<Result> list = mCachedResults->resultsAsList();
    for (int i = 0; i < list.size(); i++) {
        Result match = list.at(i);

        if (file == match.filepath() && match.lineNr() == lineNr && match.colNr() == colNr - match.length()) {
            updateNrMatches(list.indexOf(match) + 1);

            if (resultsView() && !mHasChanged)
                resultsView()->selectItem(i);

            return;
        }
    }
    updateNrMatches();
}

void SearchDialog::on_combo_search_currentTextChanged(const QString)
{
    if (!mSuppressChangeEvent)
        searchParameterChanged();
}

void SearchDialog::searchParameterChanged() {
    setSearchStatus(SearchDialog::Clear);
    invalidateCache();
}

void SearchDialog::on_cb_caseSens_stateChanged(int)
{
    searchParameterChanged();
}

void SearchDialog::updateReplaceActionAvailability()
{
    bool activateSearch = ViewHelper::editorType(mMain->recent()->editor()) == EditorType::source
                          || ViewHelper::editorType(mMain->recent()->editor()) == EditorType::txt
                          || ViewHelper::editorType(mMain->recent()->editor()) == EditorType::lxiLst
                          || ViewHelper::editorType(mMain->recent()->editor()) == EditorType::txtRo;
    activateSearch = activateSearch || (ui->combo_scope->currentIndex() != SearchScope::ThisFile);

    AbstractEdit *edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
    TextView *tm = ViewHelper::toTextView(mMain->recent()->editor());

    bool activateReplace = ((edit && !edit->isReadOnly()) || (tm && (ui->combo_scope->currentIndex() != SearchScope::ThisFile)));

    // replace actions (!readonly):
    ui->txt_replace->setEnabled(activateReplace);
    ui->btn_Replace->setEnabled(activateReplace);
    ui->btn_ReplaceAll->setEnabled(activateReplace);

    // search actions (!gdx || !lst):
    ui->combo_search->setEnabled(activateSearch);
    ui->btn_FindAll->setEnabled(activateSearch);
    ui->btn_forward->setEnabled(activateSearch);
    ui->btn_back->setEnabled(activateSearch);
    ui->btn_clear->setEnabled(activateSearch);

    ui->cb_caseSens->setEnabled(activateSearch);
    ui->cb_regex->setEnabled(activateSearch);
    ui->cb_wholeWords->setEnabled(activateSearch);

    ui->combo_filePattern->setEnabled(activateSearch && (ui->combo_scope->currentIndex() != SearchScope::ThisFile));
}

void SearchDialog::clearSearch()
{
    mSuppressChangeEvent = true;
    ui->combo_search->clearEditText();
    ui->txt_replace->clear();
    mSuppressChangeEvent = false;

    clearResults();
}

void SearchDialog::updateEditHighlighting()
{
    if (CodeEdit* ce = ViewHelper::toCodeEdit(mMain->recent()->editor()))
        ce->updateExtraSelections();
    if (TextView* tv = ViewHelper::toTextView(mMain->recent()->editor()))
        tv->updateExtraSelections();
}

void SearchDialog::clearResults()
{
    setSearchStatus(SearchStatus::Clear);

    delete mCachedResults;
    mCachedResults = nullptr;


    ProjectFileNode *fc = mMain->projectRepo()->findFileNode(mMain->recent()->editor());
    if (!fc) return;

    AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
    if (edit) {
        QTextCursor tc = edit->textCursor();
        tc.clearSelection();
        edit->setTextCursor(tc);
    }
    mMain->closeResultsPage();
    updateEditHighlighting();
}

void SearchDialog::setSearchStatus(SearchStatus status)
{
    switch (status) {
    case SearchStatus::Searching:
        ui->lbl_nrResults->setAlignment(Qt::AlignCenter);
        if (ui->lbl_nrResults->text() == QString("Searching...")) ui->lbl_nrResults->setText("Searching.  ");
        else if (ui->lbl_nrResults->text() == QString("Searching.  ")) ui->lbl_nrResults->setText("Searching.. ");
        else ui->lbl_nrResults->setText("Searching...");
        ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);
        break;
    case SearchStatus::NoResults:
        ui->lbl_nrResults->setAlignment(Qt::AlignCenter);
        ui->lbl_nrResults->setText("No results.");
        ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);
        break;
    case SearchStatus::Clear:
        ui->lbl_nrResults->setAlignment(Qt::AlignCenter);
        ui->lbl_nrResults->setText("");
        ui->lbl_nrResults->setFrameShape(QFrame::NoFrame);
        break;
    case SearchStatus::Replacing:
        ui->lbl_nrResults->setAlignment(Qt::AlignCenter);
        ui->lbl_nrResults->setText("Replacing...");
        ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);
        break;
    }
}

void SearchDialog::insertHistory()
{
    QString searchText(ui->combo_search->currentText());
    if (searchText.isEmpty()) return;

    if (ui->combo_search->findText(searchText) == -1) {
        ui->combo_search->insertItem(0, searchText);
    } else {
        bool state = mHasChanged;
        mSuppressChangeEvent = true;
        ui->combo_search->removeItem(ui->combo_search->findText(searchText));
        ui->combo_search->insertItem(0, searchText);
        ui->combo_search->setCurrentIndex(0);
        mSuppressChangeEvent = false;
        mHasChanged = state;
    }

    QString filePattern(ui->combo_filePattern->currentText());
    if (ui->combo_filePattern->findText(filePattern) == -1) {
        ui->combo_filePattern->insertItem(0, filePattern);
    } else {
        ui->combo_filePattern->removeItem(ui->combo_filePattern->findText(filePattern));
        ui->combo_filePattern->insertItem(0, filePattern);
        ui->combo_filePattern->setCurrentIndex(0);
    }
}

void SearchDialog::autofillSearchField()
{
    QWidget *widget = mMain->recent()->editor();
    ProjectAbstractNode *fsc = mMain->projectRepo()->findFileNode(widget);
    if (!fsc) return;

    if (AbstractEdit *edit = ViewHelper::toAbstractEdit(widget)) {
        if (edit->textCursor().hasSelection()) {
            ui->combo_search->insertItem(-1, edit->textCursor().selection().toPlainText());
            ui->combo_search->setCurrentIndex(0);
        } else {
            ui->combo_search->setEditText(ui->combo_search->itemText(0));
        }
    }

    if (TextView *tv = ViewHelper::toTextView(widget)) {
        if (tv->hasSelection()) {
            ui->combo_search->insertItem(-1, tv->selectedText());
            ui->combo_search->setCurrentIndex(0);
        } else {
            ui->combo_search->setEditText(ui->combo_search->itemText(0));
        }
    }

    ui->combo_search->setFocus();
    ui->combo_search->lineEdit()->selectAll();
}

void SearchDialog::updateNrMatches(int current, int max)
{
    SearchResultList* list = mCachedResults;
    int size = list ? list->size() : max;

    if (current == 0) {
        if (list->size() == 1)
            ui->lbl_nrResults->setText(QString::number(size) + " match");
        else
            ui->lbl_nrResults->setText(QString::number(size) + " matches");

        if (list->size() > MAX_SEARCH_RESULTS-1) {
            ui->lbl_nrResults->setText( QString::number(MAX_SEARCH_RESULTS) + "+ matches");
            ui->lbl_nrResults->setToolTip("Search is limited to " + QString::number(MAX_SEARCH_RESULTS) + " matches.");
        } else {
            ui->lbl_nrResults->setToolTip("");
        }

    } else
        ui->lbl_nrResults->setText(QString::number(current) + " / " + QString::number(size) + " matches");

    ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);
}

QRegularExpression SearchDialog::createRegex()
{
    QString searchTerm = ui->combo_search->currentText();
    QRegularExpression searchRegex(searchTerm);

    if (!regex()) searchRegex.setPattern(QRegularExpression::escape(searchTerm));
    if (wholeWords()) searchRegex.setPattern("\\b" + searchRegex.pattern() + "\\b");
    if (!caseSens()) searchRegex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

    return searchRegex;
}

void SearchDialog::invalidateCache()
{
    // dont delete cache when thread is running
    if (mThread.isFinished()) {
        delete mCachedResults;
        mCachedResults = nullptr;
    }
    mHasChanged = true;
    if (resultsView()) resultsView()->setOutdated();
}

bool SearchDialog::regex()
{
    return ui->cb_regex->isChecked();
}

bool SearchDialog::caseSens()
{
    return ui->cb_caseSens->isChecked();
}

bool SearchDialog::wholeWords()
{
    return ui->cb_wholeWords->isChecked();
}

QString SearchDialog::searchTerm()
{
    return ui->combo_search->currentText();
}

int SearchDialog::selectedScope()
{
    return ui->combo_scope->currentIndex();
}

void SearchDialog::setSelectedScope(int index)
{
    ui->combo_scope->setCurrentIndex(index);
}

///
/// \brief SearchDialog::results returns cached results
/// \return SearchResultList*
///
SearchResultList* SearchDialog::results()
{
    return mCachedResults;
}

ResultsView* SearchDialog::resultsView() const
{
    return mResultsView;
}

void SearchDialog::setResultsView(ResultsView* resultsView)
{
    delete mResultsView;
    mResultsView = resultsView;
}

QFlags<QTextDocument::FindFlag> SearchDialog::setFlags(SearchDirection direction)
{
    QFlags<QTextDocument::FindFlag> flags;
    flags.setFlag(QTextDocument::FindBackward, direction == SearchDirection::Backward);
    flags.setFlag(QTextDocument::FindCaseSensitively, caseSens());
    flags.setFlag(QTextDocument::FindWholeWords, wholeWords());

    return flags;
}

}
}
}
