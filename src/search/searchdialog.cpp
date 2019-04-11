/*
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

    mCachedResults = new SearchResultList();

    connect(ui->combo_search->lineEdit(), &QLineEdit::returnPressed, this, &SearchDialog::returnPressed);
}

SearchDialog::~SearchDialog()
{
    delete ui;
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
        if (createRegex().pattern().isEmpty()) return;
        mShowResults = true;

        setSearchOngoing(true);
        clearResults();

        mCachedResults->clear();
        mCachedResults->setSearchTerm(createRegex().pattern());
        mCachedResults->useRegex(regex());

        insertHistory();

        findInFiles();

        updateEditHighlighting();
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
    }

    updateEditHighlighting();

    if (!mCachedResults->size()) setSearchStatus(SearchStatus::NoResults);
    else updateFindNextLabel(QTextCursor());
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
void SearchDialog::findInFiles(QList<FileMeta*> fml, bool skipFilters)
{
    QList<FileMeta*> umodified;
    QList<FileMeta*> modified; // need to be treated differently
    QRegExp fileFilter(ui->combo_filePattern->currentText().trimmed());
    fileFilter.setPatternSyntax(QRegExp::Wildcard);

    if (fml.isEmpty() && skipFilters) fml = mMain->fileRepo()->fileMetas();
    else if (fml.isEmpty()) fml = getFilesByScope();

    for(FileMeta* fm : fml) {
        // sort files by modified
        if (fm->isModified())
            modified << fm;
        else
            umodified << fm;
    }

    // non-parallel first
    for (FileMeta* fm : modified)
        findInDoc(createRegex(), fm);

    // search file thread
    SearchWorker* sw = new SearchWorker(mMutex, createRegex(), umodified, mCachedResults);
    sw->moveToThread(&mThread);

    connect(&mThread, &QThread::finished, sw, &QObject::deleteLater, Qt::UniqueConnection);
    connect(this, &SearchDialog::startSearch, sw, &SearchWorker::findInFiles, Qt::UniqueConnection);
    connect(sw, &SearchWorker::update, this, &SearchDialog::intermediateUpdate, Qt::UniqueConnection);
    connect(sw, &SearchWorker::resultReady, this, &SearchDialog::finalUpdate, Qt::UniqueConnection);

    mThread.start();
    emit startSearch();
}

void SearchDialog::findInDoc(QRegularExpression searchRegex, FileMeta* fm)
{
    QTextCursor lastItem = QTextCursor(fm->document());
    QTextCursor item;
    QFlags<QTextDocument::FindFlag> flags = setFlags(SearchDirection::Forward);
    do {
        item = fm->document()->find(searchRegex, lastItem, flags);
        if (item != lastItem) lastItem = item;
        else break;

        if (!item.isNull()) {
            mCachedResults->addResult(item.blockNumber()+1, item.columnNumber() - item.selectedText().length(),
                                      item.selectedText().length(), fm->location(), item.block().text().trimmed());
        }
        if (mCachedResults->size() > 49000) break;
    } while (!item.isNull());
}

QList<FileMeta*> SearchDialog::getFilesByScope()
{
    QList<FileMeta*> files;
    switch (ui->combo_scope->currentIndex()) {
    case SearchScope::ThisFile:
        if (mMain->recent()->editor())
            files.append(mMain->fileRepo()->fileMeta(mMain->recent()->editor()));
        break;
    case SearchScope::ThisGroup:
        for (ProjectFileNode* fn : mMain->projectRepo()->findFileNode(mMain->recent()->editor())->parentNode()->listFiles(true)) {
            if (!files.contains(fn->file()))
                files.append(fn->file());
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
    return files;
}

void SearchDialog::replaceAll()
{
    QList<FileMeta*> fml = getFilesByScope();

    QList<FileMeta*> unmodified;
    QList<FileMeta*> modified;
    QRegExp fileFilter(ui->combo_filePattern->currentText().trimmed());
    fileFilter.setPatternSyntax(QRegExp::Wildcard);

    setSearchStatus(SearchStatus::Searching);
    int matchedFiles = 0;

    for (FileMeta* fm : fml) {

        if (fm->isReadOnly()) {
            fml.removeOne(fm);
            continue;
        }

        // check if filtered by pattern (when not SearchScope == ThisFile)
        if (ui->combo_scope->currentIndex() != SearchScope::ThisFile && fileFilter.indexIn(fm->location()) == -1) {
            fml.removeOne(fm);
            continue;
        }

        if (fm->isModified()) modified << fm;
        else unmodified << fm;

        matchedFiles++;
    }

    QString searchTerm = ui->combo_search->currentText();
    QString replaceTerm = ui->txt_replace->text();

    QMessageBox msgBox;
    if (fml.length() == 0) {
        msgBox.setText("Nothing to replace.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;

    } else if (matchedFiles == 1) {
        msgBox.setText("Are you sure you want to replace all occurences of '" +
                       searchTerm + "' with '" + replaceTerm + "' in file "
                       + fml.first()->name() + ". This action cannot be undone. Are you sure?");
    } else if (matchedFiles >= 2) {
        msgBox.setText("Are you sure you want to replace all occurences of '" +
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
    QPushButton *showCandidates = msgBox.addButton("Start Search", QMessageBox::RejectRole);
    msgBox.setDefaultButton(showCandidates);

    msgBox.exec();
    if (msgBox.clickedButton() == ok) {

        QRegularExpression regex = createRegex();
        QFlags<QTextDocument::FindFlag> flags = setFlags(SearchDirection::Forward);

        // replace using document() for modified files
        for (FileMeta* fm : modified)
            replaceOpened(fm, regex, replaceTerm, flags);

        // file-based replace for unmodified (and unopened) files
        for (FileMeta* fm : unmodified)
            replaceUnopened(fm, regex, replaceTerm);

    } else if (msgBox.clickedButton() == showCandidates) {
        findInFiles(fml);
    } else if (msgBox.clickedButton() == cancel) {
        return;
    }

    clearResults();
    invalidateCache();
}

///
/// \brief SearchDialog::replaceUnopened replaces in files where there is currently no editor open
/// \param fm file
/// \param regex find
/// \param replaceTerm replace with
///
void SearchDialog::replaceUnopened(FileMeta* fm, QRegularExpression regex, QString replaceTerm)
{
    QFile file(fm->location());
    QTextStream ts(&file);
    ts.setCodec(fm->codec());

    if (file.open(QIODevice::ReadWrite)) {
        QString content = ts.readAll();
        content.replace(regex, replaceTerm);

        ts.seek(0);
        ts << content;
    }
    file.close();

    if (fm->document()) fm->reload();
}

///
/// \brief SearchDialog::replaceOpened uses QTextDocument for replacing strings. this allows the user
/// to undo changes made by replacing.
/// \param fm filemeta
/// \param regex find
/// \param replaceTerm replace with
/// \param flags options
///
void SearchDialog::replaceOpened(FileMeta* fm, QRegularExpression regex, QString replaceTerm, QFlags<QTextDocument::FindFlag> flags)
{
    QTextCursor item;
    QTextCursor lastItem;

    QTextCursor tc;
    if (fm->editors().size() > 0)
        tc = ViewHelper::toAbstractEdit(fm->editors().first())->textCursor();

    tc.beginEditBlock();
    do {
        item = fm->document()->find(regex, lastItem, flags);
        lastItem = item;

        if (!item.isNull()) item.insertText(replaceTerm);
    } while(!item.isNull());
    tc.endEditBlock();

    if (fm->document()) fm->reload();
}

void SearchDialog::updateSearchCache()
{
    QApplication::sendPostedEvents();
    mCachedResults->clear();
    mCachedResults->setSearchTerm(createRegex().pattern());
    mCachedResults->useRegex(regex());
    findInFiles(QList<FileMeta*>() << mMain->fileRepo()->fileMeta(mMain->recent()->editor()), true);

    mHasChanged = false;
}

void SearchDialog::findNext(SearchDirection direction)
{
    if (!mMain->recent()->editor() || ui->combo_search->currentText() == "") return;

    mShowResults = false;
    setSearchOngoing(true);
    // only cache when we have changes or are not searching a large file
    if (mHasChanged && !ViewHelper::toTextView(mMain->recent()->editor()))
        updateSearchCache();

    selectNextMatch(direction);
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
        if (found) {
            setSearchStatus(SearchStatus::Clear);
        }
        if (mSplitSearchContinue) {
            setSearchStatus(SearchStatus::Searching);
            QTimer::singleShot(50, this, &SearchDialog::searchResume);
        } else {
            if (!found)
                setSearchStatus(SearchStatus::NoResults);
            mSplitSearchView = nullptr;
        }
    }
    setSearchOngoing(false);
}

void SearchDialog::on_combo_scope_currentIndexChanged(int index)
{
    ui->combo_filePattern->setEnabled(index != SearchScope::ThisFile);
    searchParameterChanged();
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

void SearchDialog::updateFindNextLabel(QTextCursor matchSelection)
{
    setSearchOngoing(false);

    if (matchSelection.isNull()) {
        AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
        if (edit) {
            matchSelection = edit->textCursor();
        } else {
            updateMatchAmount();
            return;
        }
    }

    int count = 0;
    for (Result match: mCachedResults->resultList()) {
        if (match.lineNr() == matchSelection.blockNumber()+1
                && match.colNr() == matchSelection.columnNumber() - matchSelection.selectedText().length()) {
            updateMatchAmount(count+1);
            return;
        } else {
            count++;
        }
    }
    updateMatchAmount();
}

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
                tc.movePosition(QTextCursor::End); // move to bottom
            edit->setTextCursor(tc);

            // try once more to start over
            if (!second) selectNextMatch(direction, true);
            else setSearchStatus(SearchStatus::NoResults);

        } else { // found next match
            edit->jumpTo(matchSelection);
            edit->setTextCursor(matchSelection);
        }
    } else if (TextView* tv = ViewHelper::toTextView(mMain->recent()->editor())) {
        mMain->closeResultsPage();

        mSplitSearchView = tv;
        mSplitSearchFlags = flags;
        mSplitSearchContinue = false;
        searchResume();
    }

    // set match and counter
    setSearchOngoing(false);
    setSearchStatus(SearchStatus::Clear);
    updateFindNextLabel(matchSelection);
}

void SearchDialog::on_combo_search_currentTextChanged(const QString)
{
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
    ui->combo_scope->setEnabled(activateSearch);
}

void SearchDialog::clearSearch()
{
    ui->combo_search->clearEditText();
    ui->txt_replace->clear();

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
    ProjectFileNode *fc = mMain->projectRepo()->findFileNode(mMain->recent()->editor());
    if (!fc) return;

    AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
    if (edit) {
        QTextCursor tc = edit->textCursor();
        tc.clearSelection();
        edit->setTextCursor(tc);
    }
    mCachedResults->clear();
    updateEditHighlighting();
    mMain->closeResultsPage();
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
    }
}

void SearchDialog::insertHistory()
{
    QString searchText(ui->combo_search->currentText());
    if (ui->combo_search->findText(searchText) == -1) {
        ui->combo_search->insertItem(0, searchText);
    } else {
        bool state = mHasChanged;
        ui->combo_search->removeItem(ui->combo_search->findText(searchText));
        ui->combo_search->insertItem(0, searchText);
        ui->combo_search->setCurrentIndex(0);
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

void SearchDialog::updateMatchAmount(int current)
{
    if (current == 0) {
        if (mCachedResults->size() == 1)
            ui->lbl_nrResults->setText(QString::number(mCachedResults->size()) + " match");
        else
            ui->lbl_nrResults->setText(QString::number(mCachedResults->size()) + " matches");

        if (mCachedResults->size() > 49999) {
            ui->lbl_nrResults->setText("50000+ matches");
            ui->lbl_nrResults->setToolTip("Search is limited to 50000 matches.");
        } else {
            ui->lbl_nrResults->setToolTip("");
        }

    } else {
        ui->lbl_nrResults->setText(QString::number(current) + " / " + QString::number(mCachedResults->size()) + " matches");
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

SearchResultList* SearchDialog::cachedResults()
{
    return mCachedResults;
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
