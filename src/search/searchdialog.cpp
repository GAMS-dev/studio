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
#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "settings.h"
#include "syntax.h"
#include "file.h"
#include "exception.h"
#include "searchresultmodel.h"
#include "searchworker.h"
#include "option/solveroptionwidget.h"
#include "viewhelper.h"
#include "lxiviewer/lxiviewer.h"
#include "../keys.h"

#include <QTextDocumentFragment>

namespace gams {
namespace studio {
namespace search {

SearchDialog::SearchDialog(MainWindow *parent) :
    QDialog(parent), ui(new Ui::SearchDialog), mMain(parent), mSearch(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    Settings *mSettings = Settings::settings();

    connect(&mSearch, &Search::updateLabelByCursorPos, this, &SearchDialog::updateLabelByCursorPos);

    ui->setupUi(this);
    ui->cb_regex->setChecked(mSettings->toBool(skSearchUseRegex));
    ui->cb_caseSens->setChecked(mSettings->toBool(skSearchCaseSens));
    ui->cb_wholeWords->setChecked(mSettings->toBool(skSearchWholeWords));
    ui->lbl_nrResults->setText("");
    ui->combo_search->setCompleter(nullptr);
    adjustSize();
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::on_btn_Replace_clicked()
{
    if (ui->combo_search->currentText().isEmpty()) return;
    insertHistory();

    mShowResults = false;
    mSearch.setParameters(getFilesByScope(), createRegex());
    mSearch.start();
    mSearch.replaceNext(ui->txt_replace->text());
}

void SearchDialog::on_btn_ReplaceAll_clicked()
{
    if (ui->combo_search->currentText().isEmpty()) return;
    insertHistory();

    mSearch.setParameters(getFilesByScope(), createRegex());
    mSearch.replaceAll(ui->txt_replace->text());

    searchParameterChanged();
}

void SearchDialog::on_btn_FindAll_clicked()
{
    if (!mSearch.isRunning()) {
        if (ui->combo_search->currentText().isEmpty()) return;

        updateUi(true);
        mSearch.setParameters(getFilesByScope(), createRegex());
        insertHistory();

        clearResultsView();

        mShowResults = true;
        mSearch.start();
    } else {
        mSearch.stop();
        updateUi(false);
    }
}

void SearchDialog::intermediateUpdate(int hits)
{
    setSearchStatus(Search::Searching, hits);
    QApplication::processEvents();
}

void SearchDialog::finalUpdate()
{
    updateUi(false);

    if (mShowResults) {
        if (mSearchResultModel) delete mSearchResultModel;
        mSearchResultModel = new SearchResultModel(createRegex(), mSearch.results());
        mMain->showResults(mSearchResultModel);

        mMain->resultsView()->resizeColumnsToContent();
    }

    updateEditHighlighting();

    if (mSearch.results().size() == 0)
        setSearchStatus(Search::NoResults);
    else updateLabelByCursorPos();

    mShowResults = true; // reset default
}

void SearchDialog::updateUi(bool searching)
{
    if (searching)
        ui->btn_FindAll->setText("Abort");
    else
        ui->btn_FindAll->setText("Find All");

    // deactivate actions while search is ongoing
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

    QApplication::processEvents();
}

QList<FileMeta*> SearchDialog::getFilesByScope(bool ignoreReadOnly)
{
    QList<FileMeta*> files;
    switch (ui->combo_scope->currentIndex()) {
    case Search::ThisFile:
        if (mMain->recent()->editor())
            return files << mMain->fileRepo()->fileMeta(mMain->recent()->editor());
        break;
    case Search::ThisProject:
    {
        PExFileNode* p = mMain->projectRepo()->findFileNode(mMain->recent()->editor());
        if (!p) return files;
        for (PExFileNode *c :p->assignedProject()->listFiles()) {
            if (!files.contains(c->file()))
                files.append(c->file());
        }
    }
        break;
    case Search::OpenTabs:
        files = QList<FileMeta*>::fromVector(mMain->fileRepo()->openFiles());
        break;
    case Search::AllFiles:
        files = mMain->fileRepo()->fileMetas();
        break;
    default:
        break;
    }

    // apply filter
    QStringList filter = ui->combo_filePattern->currentText().split(',', Qt::SkipEmptyParts);
    // convert user input to wildcard list
    QList<QRegExp> filterList;
    for (const QString &s : qAsConst(filter))
        filterList.append(QRegExp(s.trimmed(), Qt::CaseInsensitive, QRegExp::Wildcard));

    // filter files
    FileMeta* current = mMain->fileRepo()->fileMeta(mMain->recent()->editor());
    QList<FileMeta*> res;
    for (FileMeta* fm : qAsConst(files)) {
        bool matchesWildcard = false;

        for (const QRegExp &wildcard : filterList) {
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

void SearchDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    autofillSearchField();
    updateReplaceActionAvailability();
}

void SearchDialog::on_searchNext()
{
    on_btn_forward_clicked();
}

void SearchDialog::on_searchPrev()
{
    on_btn_back_clicked();
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
        on_btn_back_clicked();
        e->accept();
    } else if (e == Hotkey::SearchFindNext) {
        on_btn_forward_clicked();
        e->accept();
    } else if (e->modifiers() & Qt::ShiftModifier && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)) {
        e->accept();
        on_btn_FindAll_clicked();
    } else if (e == Hotkey::OpenHelp) {
        mMain->receiveOpenDoc(help::HelpData::getChapterLocation(help::DocumentType::StudioMain),
                              help::HelpData::getStudioSectionAnchor(help::HelpData::getStudioSectionName(help::StudioSection::SearchAndReplace)));
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
    if (ui->combo_search->currentText().isEmpty()) return;
    if (!getFilesByScope().contains(mMain->fileRepo()->fileMeta(mMain->recent()->editor()))) {
        setSearchStatus(Search::NoResults);
        return;
    }

    mShowResults = false;
    mSearch.setParameters(getFilesByScope(), createRegex(), true);

    insertHistory();
    mSearch.findNext(Search::Backward);
}

void SearchDialog::on_btn_forward_clicked()
{
    if (ui->combo_search->currentText().isEmpty()) return;
    if (!getFilesByScope().contains(mMain->fileRepo()->fileMeta(mMain->recent()->editor()))) {
        setSearchStatus(Search::NoResults);
        return;
    }

    mShowResults = false;
    mSearch.setParameters(getFilesByScope(), createRegex());

    insertHistory();
    mSearch.findNext(Search::Forward);
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
/// \brief SearchDialog::updateFindNextLabel calculates match number, updates label and result lists from cursor position
/// \param matchSelection cursor position to calculate result number
///
int SearchDialog::updateLabelByCursorPos(int lineNr, int colNr)
{
    updateUi(false);

    QString file = "";
    if(mMain->recent()->editor()) file = ViewHelper::location(mMain->recent()->editor());

    // if unknown get cursor from current editor
    if (lineNr == -1 || colNr == -1) {
        AbstractEdit* edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
        TextView* tv = ViewHelper::toTextView(mMain->recent()->editor());
        if (edit) {
            QTextCursor tc = edit->textCursor();
            lineNr = tc.blockNumber()+1;
            colNr = tc.positionInBlock();
        } else if (tv) {
            lineNr = tv->position().y()+1;
            colNr = tv->position().x();
        }
    }

    // find match by cursor position
    QList<Result> list = mSearch.results();
    int size = list.size() >= MAX_SEARCH_RESULTS ? MAX_SEARCH_RESULTS : list.size();
    for (int i = 0; i < size; i++) {
        Result match = list.at(i);

        if (file == match.filepath() && match.lineNr() == lineNr && match.colNr() == colNr - match.length()) {

            if (mMain->resultsView() && !mMain->resultsView()->isOutdated())
                mMain->resultsView()->selectItem(i);

            updateNrMatches(i + 1);
            return i;
        }
    }
    if (mMain->resultsView()) mMain->resultsView()->selectItem(-1);
    updateNrMatches();

    return -1;
}

void SearchDialog::on_combo_search_currentTextChanged(const QString)
{
    if (!mSuppressChangeEvent)
        searchParameterChanged();
}

void SearchDialog::searchParameterChanged() {
    setSearchStatus(Search::Clear);

    mSearch.reset();
    if (mMain->resultsView()) mMain->resultsView()->setOutdated();
}

void SearchDialog::on_cb_caseSens_stateChanged(int)
{
    searchParameterChanged();
}

// TODO(RG): merge this with updateUi
void SearchDialog::updateReplaceActionAvailability()
{
    bool activateSearch = ViewHelper::editorType(mMain->recent()->editor()) == EditorType::source
                          || ViewHelper::editorType(mMain->recent()->editor()) == EditorType::txt
                          || ViewHelper::editorType(mMain->recent()->editor()) == EditorType::lxiLstChild
                          || ViewHelper::editorType(mMain->recent()->editor()) == EditorType::txtRo;
    activateSearch = activateSearch || (ui->combo_scope->currentIndex() != Search::ThisFile);

    AbstractEdit *edit = ViewHelper::toAbstractEdit(mMain->recent()->editor());
    TextView *tm = ViewHelper::toTextView(mMain->recent()->editor());

    bool activateReplace = ((edit && !edit->isReadOnly()) ||
                            (tm && (ui->combo_scope->currentIndex() != Search::ThisFile)));

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

    ui->combo_filePattern->setEnabled(activateSearch && (ui->combo_scope->currentIndex() != Search::ThisFile));
}

void SearchDialog::clearSearch()
{
    mSuppressChangeEvent = true;
    ui->combo_search->clearEditText();
    ui->txt_replace->clear();
    mSuppressChangeEvent = false;
    mSearch.reset();
    mSearch.setParameters(QList<FileMeta*>(), QRegularExpression(""));

    if (CodeEdit* ce = ViewHelper::toCodeEdit(mMain->recent()->editor())) {
        QTextCursor tc = ce->textCursor();
        tc.clearSelection();
        ce->setTextCursor(tc);
    } else if (TextView* tv = ViewHelper::toTextView(mMain->recent()->editor())) {
        QTextCursor tc = tv->edit()->textCursor();
        tc.clearSelection();
        tv->edit()->setTextCursor(tc);
    }

    clearResultsView();
    updateEditHighlighting();
}

void SearchDialog::updateEditHighlighting()
{
    if (AbstractEdit* ae = ViewHelper::toCodeEdit(mMain->recent()->editor()))
        ae->updateExtraSelections();
    else if (TextView* tv = ViewHelper::toTextView(mMain->recent()->editor()))
        tv->updateExtraSelections();
}

void SearchDialog::clearResultsView()
{
    setSearchStatus(Search::Clear);
    mMain->closeResultsPage();
}

void SearchDialog::setSearchStatus(Search::Status status, int hits)
{
    QString searching = "Searching (";
    QString dotAnim = ".";
    QRegularExpression re("\\.*");

    hits = (hits > MAX_SEARCH_RESULTS-1) ? MAX_SEARCH_RESULTS : hits;

    switch (status) {
    case Search::Searching:
        ui->lbl_nrResults->setAlignment(Qt::AlignCenter);
        ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);

        ui->lbl_nrResults->setText(searching + QString::number(hits) + ") "
                                   + dotAnim.repeated(mSearchAnimation++ % 4));
        break;
    case Search::NoResults:
        ui->lbl_nrResults->setAlignment(Qt::AlignCenter);
        ui->lbl_nrResults->setText("No results.");
        ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);
        break;
    case Search::Clear:
        ui->lbl_nrResults->setAlignment(Qt::AlignCenter);
        ui->lbl_nrResults->setText("");
        ui->lbl_nrResults->setFrameShape(QFrame::NoFrame);
        break;
    case Search::Replacing:
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
        mSuppressChangeEvent = true;
        ui->combo_search->removeItem(ui->combo_search->findText(searchText));
        ui->combo_search->insertItem(0, searchText);
        ui->combo_search->setCurrentIndex(0);
        mSuppressChangeEvent = false;
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
    PExAbstractNode *fsc = mMain->projectRepo()->findFileNode(widget);
    if (!fsc) return;

    if (AbstractEdit *edit = ViewHelper::toAbstractEdit(widget)) {
        if (edit->textCursor().hasSelection()) {
            ui->combo_search->insertItem(-1, edit->textCursor().selection().toPlainText());
            ui->combo_search->setCurrentIndex(0);
        } else {
            QString text;
            if (CodeEdit *ce = ViewHelper::toCodeEdit(widget))
                text = ce->wordUnderCursor();
            if (text.isEmpty()) text = ui->combo_search->itemText(0);
            ui->combo_search->setEditText(text);
        }
    }

    if (TextView *tv = ViewHelper::toTextView(widget)) {
        if (tv->hasSelection()) {
            ui->combo_search->insertItem(-1, tv->selectedText());
            ui->combo_search->setCurrentIndex(0);
        } else {
            QString text = tv->wordUnderCursor();
            if (text.isEmpty()) text = ui->combo_search->itemText(0);
            ui->combo_search->setEditText(text);
        }
    }

    ui->combo_search->setFocus();
    ui->combo_search->lineEdit()->selectAll();
}

void SearchDialog::updateNrMatches(int current)
{
    int size = (mSearch.results().size() >= MAX_SEARCH_RESULTS)
            ? MAX_SEARCH_RESULTS : mSearch.results().size();

    if (current == 0) {
        if (size == 1) {
            ui->lbl_nrResults->setText(QString::number(size) + " match");

        } else {
            if (size == 0) setSearchStatus(Search::NoResults);
            else ui->lbl_nrResults->setText(QString::number(size) + " matches");
        }

        if (size >= MAX_SEARCH_RESULTS) {
            ui->lbl_nrResults->setText( QString::number(MAX_SEARCH_RESULTS) + "+ matches");
            ui->lbl_nrResults->setToolTip("Search is limited to " + QString::number(MAX_SEARCH_RESULTS) + " matches.");
        } else {
            ui->lbl_nrResults->setToolTip("");
        }

    } else {
        QString plus("");
        if (size >= MAX_SEARCH_RESULTS) plus = "+";
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

Search* SearchDialog::search()
{
    return &mSearch;
}

}
}
}
