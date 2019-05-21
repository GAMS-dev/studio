﻿/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "../studiosettings.h"
#include "../syntax.h"
#include "../file.h"
#include "../exception.h"
#include "searchresultlist.h"
#include "searchworker.h"
#include "locators/settingslocator.h"
#include "editors/viewhelper.h"

#include <QMessageBox>
#include <QTextDocumentFragment>
#include <QThread>

namespace gams {
namespace studio {

SearchDialog::SearchDialog(MainWindow *parent) :
    QDialog(parent), ui(new Ui::SearchDialog), mMain(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    StudioSettings *mSettings = SettingsLocator::settings();

    ui->setupUi(this);
    ui->cb_regex->setChecked(mSettings->searchUseRegex());
    ui->cb_caseSens->setChecked(mSettings->searchCaseSens());
    ui->cb_wholeWords->setChecked(mSettings->searchWholeWords());
    ui->combo_scope->setCurrentIndex(mSettings->selectedScopeIndex());
    ui->lbl_nrResults->setText("");
    ui->combo_search->setAutoCompletion(false);
    adjustSize();

    connect(ui->combo_search->lineEdit(), &QLineEdit::returnPressed, this, &SearchDialog::returnPressed);
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

    QString replaceTerm = ui->txt_replace->text();
    if (edit->textCursor().hasSelection()) {
        edit->textCursor().insertText(replaceTerm);
        invalidateCache();
    }

    findNext(SearchDialog::Forward);
}

void SearchDialog::on_btn_ReplaceAll_clicked()
{
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
        findInFiles(mCachedResults);
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
        mFinalResults = mCachedResults;
        mCachedResults = nullptr;
        mMain->showResults(mFinalResults);
        resultsView()->resizeColumnsToContent();
    }

    updateEditHighlighting();

    if (mFinalResults && !mFinalResults->size()) setSearchStatus(SearchStatus::NoResults);
    else updateFindNextLabel(0, 0);
}

void SearchDialog::setSearchOngoing(bool searching)
{
    mSearching = searching;

    if (searching) {
        ui->btn_FindAll->setText("Abort");
    } else {
        ui->btn_FindAll->setText("Find All");
    }
}

///
/// \brief SearchDialog::findInFiles starts a search in files. distinguishes between (non-)modified files.
/// \param mMutex for parallelisation
/// \param fml list of files to search
/// \param skipFilters enable for result caching (find next/prev)
///
void SearchDialog::findInFiles(SearchResultList* collection, QList<FileMeta*> fml, bool skipFilters)
{
    QList<FileMeta*> umodified;
    QList<FileMeta*> modified; // need to be treated differently

    if (fml.isEmpty() && skipFilters) fml = mMain->fileRepo()->fileMetas();
    else if (fml.isEmpty()) fml = getFilesByScope();

    for(FileMeta* fm : fml) {

        // skip certain file types
        if (fm->kind() == FileKind::Gdx || fm->kind() == FileKind::Lxi || fm->kind() == FileKind::Ref)
            continue;

        // sort files by modified
        if (fm->isModified())
            modified << fm;
        else
            umodified << fm;
    }


    // non-parallel first
    for (FileMeta* fm : modified)
        findInDoc(createRegex(), fm, collection);

    SearchWorker* sw = new SearchWorker(mMutex, createRegex(), umodified, collection);
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

QList<FileMeta*> SearchDialog::getFilesByScope()
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

    auto file = mMain->fileRepo()->openFiles();

    // apply filter
    QList<FileMeta*> res;
    for (FileMeta* fm : files) {
        if (ui->combo_scope->currentIndex() == SearchScope::ThisFile || fileFilter.indexIn(fm->location()) != -1)
            res.append(fm);
    }
    return res;
}

void SearchDialog::replaceAll()
{
    if (ui->combo_search->currentText().isEmpty()) return;

    QList<FileMeta*> fml = getFilesByScope();

    QList<FileMeta*> opened;
    QList<FileMeta*> unopened;

    int matchedFiles = 0;

    mHasChanged = false;

    // sort and filter FMs by editability and open state
    for (FileMeta* fm : fml) {
        // check if filtered by pattern (when not SearchScope == ThisFile)
        if (fm->isReadOnly()) {
            fml.removeOne(fm);
            continue;
        }

        if (fm->document()) {
            if (!opened.contains(fm))
                opened << fm;
        } else {
            if (!unopened.contains(fm))
                unopened << fm;
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
                            "This is just a representation of the selected scope in the search window.");

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

void SearchDialog::updateSearchCache()
{
    QApplication::sendPostedEvents();

    if (mCachedResults) delete mCachedResults;
    mCachedResults = new SearchResultList(createRegex());

    mShowResults = false;
    findInFiles(mCachedResults, getFilesByScope(), true);
    mHasChanged = false;
}

void SearchDialog::findNext(SearchDirection direction)
{
    if (ui->combo_search->currentText() == "") return;

    if (!mCachedResults || mHasChanged || mCachedResults->filteredResultList(mMain->recent()->editor()->property("location").toString()).isEmpty())
        updateSearchCache();

    selectNextMatch(direction);

// TODO(rogo): remove two find next modes
//    if (mResultsView && !mHasChanged) {
//        QWidget* edit = mMain->recent()->editor();

//        int line = -1;
//        int col = -1;

//        if (CodeEdit* ce = ViewHelper::toCodeEdit(edit)) {
//            line = ce->textCursor().blockNumber()+1;
//            col = ce->textCursor().positionInBlock();
//            QTextCursor tc = ce->textCursor();
//            tc.clearSelection();
//            ce->setTextCursor(tc);
//        } else if (TextView* tv = ViewHelper::toTextView(edit)) {
//            line = tv->position().y()+1;
//            col = tv->position().x();
//            QTextCursor tc = tv->edit()->textCursor();
//            tc.clearSelection();
//            tv->edit()->setTextCursor(tc);
//        }

//        int selection = mResultsView->selectNextItem(ViewHelper::location(edit), line, col, direction);

//        if (selection != -1) {
//            updateMatchLabel(selection);
//        } else{
//            // if reached maximum index but there are more hits
//            selectNextMatch(direction);
//        }

//    } else {
//    }
}

///
/// \brief SearchDialog::selectNextMatch steps through words in a document
/// \param direction
/// \param second is second time entering this function, to avoid too deep recursion
///
void SearchDialog::selectNextMatch(SearchDirection direction, bool second)
{
    QTextCursor matchSelection;
    QRegularExpression searchRegex = createRegex();

    setSearchStatus(SearchStatus::Searching);

    ProjectFileNode *fc = mMain->projectRepo()->findFileNode(mMain->recent()->editor());
    if (!fc) return;
    QFlags<QTextDocument::FindFlag> flags = setFlags(direction);

    if (AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor())) {
        matchSelection = fc->document()->find(searchRegex, edit->textCursor(), flags);

        if (matchSelection.isNull()) { // empty selection == reached end of document

            QTextCursor tc(edit->document()); // set to top
            if (direction == SearchDirection::Backward)
                tc.movePosition(QTextCursor::End); // if reverse, move to bottom
            edit->setTextCursor(tc);

            // try once more to start over
            if (!second) selectNextMatch(direction, true);
            else setSearchStatus(SearchStatus::NoResults);

        } else { // found next match
            edit->jumpTo(matchSelection);
            edit->setTextCursor(matchSelection);
        }
        updateFindNextLabel(matchSelection.blockNumber()+1, matchSelection.columnNumber());
    } else if (TextView* tv = ViewHelper::toTextView(mMain->recent()->editor())) {

        mSplitSearchView = tv;
        mSplitSearchFlags = flags;
        mSplitSearchContinue = false;
        searchResume();
    }
}

void SearchDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    mFirstReturn = true;
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
    //TODO: make smarter
    Q_UNUSED(from); Q_UNUSED(charsRemoved); Q_UNUSED(charsAdded);
    searchParameterChanged();
}

void SearchDialog::keyPressEvent(QKeyEvent* e)
{
    if ( isVisible() && ((e->key() == Qt::Key_Escape) || (e->modifiers() & Qt::ControlModifier && (e->key() == Qt::Key_F))) ) {
        e->accept();
        hide();
        if (mMain->projectRepo()->findFileNode(mMain->recent()->editor()))
            mMain->recent()->editor()->setFocus();

    } else if (e->modifiers() & Qt::ShiftModifier && (e->key() == Qt::Key_F3)) {
        e->accept();
        on_searchPrev();
    } else if (e->key() == Qt::Key_F3) {
        e->accept();
        on_searchNext();
    } else if (e->modifiers() & Qt::ShiftModifier && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)) {
        e->accept();
        on_btn_FindAll_clicked();
    }
    QDialog::keyPressEvent(e);
}

// this is a workaround for the QLineEdit field swallowing the first enter after a show event
// leading to a search using the last search term instead of the current.
void SearchDialog::returnPressed() {
    if (mFirstReturn) {
        findNext(SearchDialog::Forward);
        mFirstReturn = false;
    }
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
    Q_UNUSED(arg1);
    searchParameterChanged();
}

void SearchDialog::on_cb_regex_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    searchParameterChanged();
}

///
/// \brief SearchDialog::updateFindNextLabel counts from selection to results list and updates label
/// \param matchSelection cursor position to calculate result number
///
void SearchDialog::updateFindNextLabel(int lineNr, int colNr)
{
    setSearchOngoing(false);

    QString file = ViewHelper::location(mMain->recent()->editor());

    if (lineNr == 0 || colNr == 0) {
        AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
        TextView* tv = ViewHelper::toTextView(mMain->recent()->editor());
        QTextCursor tc;
        if (edit) tc = edit->textCursor();
        else if (tv) tc = tv->edit()->textCursor();

        lineNr = tc.blockNumber()+1;
        colNr = tc.columnNumber();
    }

    // TODO(rogo): performance improvements possible? replace mCR->rL with mCR->rH and iterate only once
    QList<Result> list = results()->resultsAsList();
    for (int i = 0; i < list.size(); i++) {
        Result match = list.at(i);

        if (file == match.filepath() && match.lineNr() == lineNr && match.colNr() == colNr - match.length()) {
            updateNrMatches(list.indexOf(match) + 1);
            // TODO(rogo): select match here
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
    bool activateReplace = (edit && !edit->isReadOnly());

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
    if (CodeEdit* ce = ViewHelper::toCodeEdit(mActiveEdit)) ce->updateExtraSelections();
    if (TextView* tv = ViewHelper::toTextView(mActiveEdit)) tv->updateExtraSelections();
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
            mFirstReturn = false;
        }
    }

    if (TextView *tv = ViewHelper::toTextView(widget)) {
        if (tv->hasSelection()) {
            ui->combo_search->insertItem(-1, tv->selectedText());
            ui->combo_search->setCurrentIndex(0);
        } else {
            ui->combo_search->setEditText(ui->combo_search->itemText(0));
            mFirstReturn = false;
        }
    }

    ui->combo_search->setFocus();
}

void SearchDialog::updateNrMatches(int current)
{
    SearchResultList* list = results();

    if (current == 0) {
        if (list->size() == 1)
            ui->lbl_nrResults->setText(QString::number(list->size()) + " match");
        else
            ui->lbl_nrResults->setText(QString::number(list->size()) + " matches");

        if (list->size() > MAX_SEARCH_RESULTS-1) {
            ui->lbl_nrResults->setText( QString::number(MAX_SEARCH_RESULTS) + "+ matches");
            ui->lbl_nrResults->setToolTip("Search is limited to " + QString::number(MAX_SEARCH_RESULTS) + " matches.");
        } else {
            ui->lbl_nrResults->setToolTip("");
        }

    } else {
        int max = list->size();
        ui->lbl_nrResults->setText(QString::number(current) + " / " + QString::number(max) + " matches");
    }

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
    delete mCachedResults;
    mCachedResults = nullptr;
    mHasChanged = true;
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
/// \brief SearchDialog::results returns either cache or if available (and up to date) final results
/// \return SearchResultList*
///
SearchResultList* SearchDialog::results()
{
    return resultsView() ? resultsView()->searchResultList() : mCachedResults;
}

void SearchDialog::setActiveEditWidget(QWidget *edit)
{
    mActiveEdit = edit;
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
