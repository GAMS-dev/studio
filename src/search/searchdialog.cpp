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
#include "locators/settingslocator.h"
#include "editors/viewhelper.h"

#include <QMessageBox>
#include <QTextDocumentFragment>

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
    // TODO: allow users to replace in more than the current file?
    simpleReplaceAll();
}

void SearchDialog::on_btn_FindAll_clicked()
{
    clearResults();

    mCachedResults.clear();
    mCachedResults.setSearchTerm(createRegex().pattern());
    mCachedResults.useRegex(regex());

    insertHistory();

    setSearchStatus(SearchStatus::Searching);
    QApplication::sendPostedEvents();

    switch (ui->combo_scope->currentIndex()) {
    case SearchScope::ThisFile:
        if (mMain->recent()->editor())
            mCachedResults.addResultList(findInFile(mMain->fileRepo()->fileMeta(mMain->recent()->editor())));
        break;
    case SearchScope::ThisGroup:
        mCachedResults.addResultList(findInGroup());
        break;
    case SearchScope::OpenTabs:
        mCachedResults.addResultList(findInOpenFiles());
        break;
    case SearchScope::AllFiles:
        mCachedResults.addResultList(findInAllFiles());
        break;
    default:
        break;
    }
    updateMatchAmount(mCachedResults.size());
    mMain->showResults(mCachedResults);

    if (CodeEdit* ce = ViewHelper::toCodeEdit(mActiveEdit)) ce->updateExtraSelections();
    if (TextView* tv = ViewHelper::toTextView(mActiveEdit)) tv->updateExtraSelections();
}

QList<Result> SearchDialog::findInFiles(QList<FileMeta*> fml, bool skipFilters)
{
    QList<Result> res;
    for (FileMeta* fm : fml)
        res << findInFile(fm, skipFilters);

    return res;
}

QList<Result> SearchDialog::findInAllFiles()
{
    QList<FileMeta*> files = mMain->fileRepo()->fileMetas();
    QList<Result> matches = findInFiles(files);
    return matches;
}

QList<Result> SearchDialog::findInOpenFiles()
{
    QList<FileMeta*> files = QList<FileMeta*>::fromVector(mMain->fileRepo()->openFiles());
    QList<Result> matches = findInFiles(files);
    return matches;
}

QList<Result> SearchDialog::findInGroup()
{
    ProjectFileNode* fc = mMain->projectRepo()->findFileNode(mMain->recent()->editor());
    ProjectGroupNode* group = (fc ? fc->parentNode() : nullptr);

    QList<FileMeta*> files;
    for (ProjectFileNode* fn : group->listFiles(true)) {
        if (!files.contains(fn->file()))
            files.append(fn->file());
    }
    QList<Result> matches = findInFiles(files);

    return matches;
}

QList<Result> SearchDialog::findInFile(FileMeta* fm, bool skipFilters)
{
    if (!fm) return QList<Result>();

    QRegExp fileFilter(ui->combo_filePattern->currentText().trimmed());
    fileFilter.setPatternSyntax(QRegExp::Wildcard);
    if (!skipFilters) {
        if (((ui->combo_scope->currentIndex() != SearchScope::ThisFile) && (fileFilter.indexIn(fm->location()) == -1))
                || (fm->kind() == FileKind::Gdx) || (fm->kind() == FileKind::Log)|| (fm->kind() == FileKind::Ref)) {
            return QList<Result>(); // dont search here, return empty
        }
    }

    QString searchTerm = ui->combo_search->currentText();
    if (searchTerm.isEmpty()) return QList<Result>();

    QRegularExpression regexp = createRegex();
    SearchResultList matches;

    // when a file has unsaved changes a different search strategy is used.
    if (fm->isModified())
        findInDoc(regexp, fm, &matches);
    else
        findOnDisk(regexp, fm, &matches);

    return matches.resultList();
}

void SearchDialog::findOnDisk(QRegularExpression searchRegex, FileMeta* fm, SearchResultList* matches)
{
    int lineCounter = 0;
    QFile file(fm->location());
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) { // read file
            lineCounter++;
            QString line = in.readLine();

            QRegularExpressionMatch match;
            QRegularExpressionMatchIterator i = searchRegex.globalMatch(line);
            while (i.hasNext()) {
                match = i.next();
                matches->addResult(lineCounter, match.capturedStart(), match.capturedLength(),
                                   file.fileName(), line.trimmed());
            }
        }
        file.close();
    }
}

void SearchDialog::findInDoc(QRegularExpression searchRegex, FileMeta* fm, SearchResultList* matches)
{
    QTextCursor lastItem = QTextCursor(fm->document());
    QTextCursor item;
    QFlags<QTextDocument::FindFlag> flags = setFlags(SearchDirection::Forward);
    do {
        item = fm->document()->find(searchRegex, lastItem, flags);
        if (item != lastItem) lastItem = item;
        else break;

        if (!item.isNull()) {
            matches->addResult(item.blockNumber()+1, item.columnNumber() - item.selectedText().length(),
                              item.selectedText().length(), fm->location(), item.block().text().trimmed());
        }
    } while (!item.isNull());
}

void SearchDialog::simpleReplaceAll()
{
    AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
    if (!edit || edit->isReadOnly()) return;

    QString searchTerm = ui->combo_search->currentText();
    if (searchTerm.isEmpty()) return;

    QRegularExpression searchRegex = createRegex();
    QString replaceTerm = ui->txt_replace->text();

    QList<QTextCursor> hits;
    QTextCursor item;
    QTextCursor lastItem;

    QFlags<QTextDocument::FindFlag> flags = setFlags(SearchDirection::Forward);

    do {
        item = edit->document()->find(searchRegex, lastItem, flags);
        lastItem = item;

        if (!item.isNull())
            hits.append(item);

    } while (!item.isNull());

    QMessageBox msgBox;
    if (hits.length() == 1) {
        msgBox.setText("Replacing 1 occurrence of '" + searchTerm + "' with '" + replaceTerm + "' in file "
                       + mMain->projectRepo()->findFileNode(mMain->recent()->editor())->location()
                       + ". Are you sure?");
    } else {
        msgBox.setText("Replacing " + QString::number(hits.length()) + " occurrences of '" +
                       searchTerm + "' with '" + replaceTerm + "' in file "
                       + mMain->projectRepo()->findFileNode(mMain->recent()->editor())->location()
                       + ". Are you sure?");
    }
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int answer = msgBox.exec();

    if (answer == QMessageBox::Ok) {
        clearResults();
        edit->textCursor().beginEditBlock();
        for (QTextCursor tc: hits) {
            tc.insertText(replaceTerm);
        }
        edit->textCursor().endEditBlock();
        invalidateCache();
    }
}

void SearchDialog::updateSearchResults()
{
    setSearchStatus(SearchStatus::Searching);
    QApplication::sendPostedEvents();
    mCachedResults.clear();
    mCachedResults.addResultList(findInFile(mMain->fileRepo()->fileMeta(mMain->recent()->editor()), true));
    mCachedResults.setSearchTerm(createRegex().pattern());
    mCachedResults.useRegex(regex());
    mHasChanged = false;
}

void SearchDialog::findNext(SearchDirection direction)
{
    if (!mMain->recent()->editor() || ui->combo_search->currentText() == "") return;

    if (mHasChanged) updateSearchResults();

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
    clearResults();
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

void SearchDialog::closeEvent(QCloseEvent *e) {
    setSearchStatus(SearchStatus::Clear);
    QDialog::closeEvent(e);
}

void SearchDialog::on_combo_scope_currentIndexChanged(int index)
{
    ui->combo_filePattern->setEnabled(index != SearchScope::ThisFile);
    searchParameterChanged();
}

void SearchDialog::on_btn_back_clicked()
{
    insertHistory();
    if (mHasChanged) clearResults();
    findNext(SearchDialog::Backward);
}

void SearchDialog::on_btn_forward_clicked()
{
    insertHistory();
    if (mHasChanged) clearResults();
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

void SearchDialog::selectNextMatch(SearchDirection direction, bool second)
{
    QPoint matchPos;
    QRegularExpression searchRegex = createRegex();

    ProjectFileNode *fc = mMain->projectRepo()->findFileNode(mMain->recent()->editor());
    if (!fc) return;
    QFlags<QTextDocument::FindFlag> flags = setFlags(direction);

    if (AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor())) {
        QTextCursor matchSelection = fc->document()->find(searchRegex, edit->textCursor(), flags);

        if (mCachedResults.size() > 0) { // has any matches at all

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
                matchPos.setY(matchSelection.blockNumber()+1);
                matchPos.setX(matchSelection.columnNumber() - matchSelection.selectedText().length());
            }
        } else { // search had no matches so do nothing
            setSearchStatus(SearchStatus::NoResults);
            QTextCursor tc = edit->textCursor();
            tc.clearSelection();
            edit->setTextCursor(tc);
            return;
        }
    }

    if (TextView* tv = ViewHelper::toTextView(mMain->recent()->editor())) {
        bool found = tv->findText(searchRegex, flags);
        if (found) {
            matchPos = tv->position();
        }
    }

    // set match and counter
    int count = 0;
    for (Result match: mCachedResults.resultList()) {
        if (match.lineNr() == matchPos.y()
                && match.colNr() == matchPos.x()) {
            updateMatchAmount(mCachedResults.size(), count+1);
            break;
        } else {
            count++;
        }
    }
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
    AbstractEdit *edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
    bool isSourceCode = (ViewHelper::editorType(mMain->recent()->editor()) == EditorType::source
                         || ViewHelper::editorType(mMain->recent()->editor()) == EditorType::txt);

    bool activateSearch = isSourceCode || ViewHelper::editorType(mMain->recent()->editor()) == EditorType::lxiLst;
    bool activateReplace = (isSourceCode && !edit->isReadOnly());

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
    mMain->closeResultsPage();
}

void SearchDialog::clearResults()
{
    ProjectFileNode *fc = mMain->projectRepo()->findFileNode(mMain->recent()->editor());
    if (!fc) return;
    setSearchStatus(SearchStatus::Clear);

    AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
    if (edit) {
        QTextCursor tc = edit->textCursor();
        tc.clearSelection();
        edit->setTextCursor(tc);
    }
    mCachedResults.clear();

    if (CodeEdit* ce = ViewHelper::toCodeEdit(mActiveEdit)) ce->updateExtraSelections();
    if (TextView* tv = ViewHelper::toTextView(mActiveEdit)) tv->updateExtraSelections();
}

void SearchDialog::setSearchStatus(SearchStatus status)
{
    switch (status) {
    case SearchStatus::Searching:
        ui->lbl_nrResults->setText("Searching...");
        ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);
        break;
    case SearchStatus::NoResults:
        ui->lbl_nrResults->setText("No results.");
        ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);
        break;
    case SearchStatus::Clear:
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

void SearchDialog::updateMatchAmount(int hits, int current)
{
    if (current == 0) {
        if (hits == 1)
            ui->lbl_nrResults->setText(QString::number(hits) + " match");
        else
            ui->lbl_nrResults->setText(QString::number(hits) + " matches");
    } else {
        ui->lbl_nrResults->setText(QString::number(current) + " / " + QString::number(hits) + " matches");
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
    return &mCachedResults;
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

    return flags;
}

}
}
