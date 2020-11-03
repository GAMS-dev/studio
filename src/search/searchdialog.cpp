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
#include "viewhelper.h"
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
    Settings *mSettings = Settings::settings();

    ui->setupUi(this);
    ui->cb_regex->setChecked(mSettings->toBool(skSearchUseRegex));
    ui->cb_caseSens->setChecked(mSettings->toBool(skSearchCaseSens));
    ui->cb_wholeWords->setChecked(mSettings->toBool(skSearchWholeWords));
    ui->combo_scope->setCurrentIndex(mSettings->toInt(skSearchScope));
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
    QRegularExpression regex = createRegex();
    QRegularExpressionMatch match = regex.match(edit->textCursor().selectedText());

    if (edit->textCursor().hasSelection() && match.hasMatch() &&
            match.captured(0) == edit->textCursor().selectedText()) { // TODO(RG): test this
        edit->textCursor().insertText(ui->txt_replace->text());
    }

    Search::selectNextMatch();
}

void SearchDialog::on_btn_ReplaceAll_clicked()
{
    insertHistory();
    Search::replaceAll();
}

void SearchDialog::on_btn_FindAll_clicked()
{
    if (!mSearching) {
        if (ui->combo_search->currentText().isEmpty()) return;

        mHasChanged = false;
        setSearchOngoing(true);
        clearResults();
        insertHistory();

        Search::start();
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
    else updateLabelByCursorPos();
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

    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

QList<FileMeta*> SearchDialog::getFilesByScope(bool ignoreReadOnly)
{
    QList<FileMeta*> files;
    switch (ui->combo_scope->currentIndex()) {
    case SearchScope::ThisFile:
        if (mMain->recent()->editor())
            return files << mMain->fileRepo()->fileMeta(mMain->recent()->editor());
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
    QStringList filter = ui->combo_filePattern->currentText().split(',', QString::SkipEmptyParts);
    // convert user input to wildcard list
    QList<QRegExp> filterList;
    for (QString s : filter)
        filterList.append(QRegExp(s.trimmed(), Qt::CaseInsensitive, QRegExp::Wildcard));

    // filter files
    FileMeta* current = mMain->fileRepo()->fileMeta(mMain->recent()->editor());
    QList<FileMeta*> res;
    for (FileMeta* fm : files) {
        bool matchesWildcard = false;

        for (QRegExp wildcard : filterList) {
            matchesWildcard = wildcard.indexIn(fm->location()) != -1;
            if (matchesWildcard) break; // one match is enough, dont overwrite result
        }

        if (matchesWildcard && (!ignoreReadOnly || !fm->isReadOnly())) {
            if (fm == current) res.insert(0, fm); // search current file first
            else res.append(fm);
        }
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

void SearchDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    autofillSearchField();
    updateReplaceActionAvailability();
}

void SearchDialog::on_searchNext()
{
    if (ui->combo_search->currentText() != "")
        findNext(SearchDialog::Forward);
}

void SearchDialog::on_searchPrev()
{
    if (ui->combo_search->currentText() != "")
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
void SearchDialog::updateLabelByCursorPos(int lineNr, int colNr)
{
    setSearchOngoing(false);

    QString file = "";
    if(mMain->recent()->editor()) file = ViewHelper::location(mMain->recent()->editor());

    // if unknown get cursor from current editor
    if (lineNr == 0 || colNr == 0) {
        AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
        TextView* tv = ViewHelper::toTextView(mMain->recent()->editor());
        if (edit) {
            QTextCursor tc = edit->textCursor();
            lineNr = tc.blockNumber()+1;
            colNr = tc.columnNumber();
        } else if (tv) {
            lineNr = tv->position().y()+1;
            colNr = tv->position().x();
        }
    }

    // find match by cursor position
    QList<Result> list = mCachedResults->resultsAsList();
    for (int i = 0; i < list.size(); i++) {
        Result match = list.at(i);

        if (file == match.filepath() && match.lineNr() == lineNr && match.colNr() == colNr - match.length()) {
            mOutsideOfList = false;

            if (resultsView() && !mHasChanged)
                resultsView()->selectItem(i);

            updateNrMatches(list.indexOf(match) + 1);
            return;
        }
    }
    if (resultsView()) resultsView()->selectItem(-1);
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

void SearchDialog::updateNrMatches(int current)
{
    SearchResultList* list = mCachedResults;
    int size = list->size();

    if (current == 0) {
        if (list->size() == 1)
            ui->lbl_nrResults->setText(QString::number(size) + " match");
        else
            ui->lbl_nrResults->setText(QString::number(size) + " matches");

        if (list->size() >= MAX_SEARCH_RESULTS) {
            ui->lbl_nrResults->setText( QString::number(MAX_SEARCH_RESULTS) + "+ matches");
            ui->lbl_nrResults->setToolTip("Search is limited to " + QString::number(MAX_SEARCH_RESULTS) + " matches.");
        } else {
            ui->lbl_nrResults->setToolTip("");
        }

    } else {
        QString plus("");
        if (list->size() >= MAX_SEARCH_RESULTS) plus = "+";
        ui->lbl_nrResults->setText(QString::number(current) + " / " + QString::number(size) + plus);
    }

    ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);
}

QRegularExpression SearchDialog::createRegex()
{
    QString searchTerm = ui->combo_search->currentText();
    QRegularExpression searchRegex(searchTerm);

    if (!regex()) searchRegex.setPattern(QRegularExpression::escape(searchTerm));
    if (wholeWords()) searchRegex.setPattern("(?<!\\w)" + searchRegex.pattern() + "(?!\\w)");
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
