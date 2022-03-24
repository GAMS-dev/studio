/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include <QFileDialog>
#include <QTextDocumentFragment>
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
#include "help/helpdata.h"

namespace gams {
namespace studio {
namespace search {

SearchDialog::SearchDialog(AbstractSearchFileHandler* fileHandler, QWidget* parent) :
    QDialog(parent), ui(new Ui::SearchDialog), mFileHandler(fileHandler), mSearch(this, fileHandler)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);

    restoreSettings();
    connect(&mSearch, &Search::updateUI, this, &SearchDialog::updateUi);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::restoreSettings()
{
    Settings *settings = Settings::settings();
    ui->cb_regex->setChecked(settings->toBool(skSearchUseRegex));
    ui->cb_caseSens->setChecked(settings->toBool(skSearchCaseSens));
    ui->cb_wholeWords->setChecked(settings->toBool(skSearchWholeWords));
    ui->lbl_nrResults->setText("");
    ui->combo_search->setCompleter(nullptr);
}

void SearchDialog::editorChanged(QWidget* editor) {
    mCurrentEditor = editor;
    mSearch.activeFileChanged();
}

void SearchDialog::on_btn_Replace_clicked()
{
    if (ui->combo_search->currentText().isEmpty()) return;
    insertHistory();

    mShowResults = false;
    mSearch.setParameters(getFilesByScope(true), createRegex());
    mSearch.start();
    mSearch.replaceNext(ui->txt_replace->text());
}

void SearchDialog::on_btn_ReplaceAll_clicked()
{
    if (ui->combo_search->currentText().isEmpty()) return;

    clearResultsView();
    insertHistory();

    mShowResults = true;
    mSearch.setParameters(getFilesByScope(true), createRegex());
    mSearch.replaceAll(ui->txt_replace->text());
}

void SearchDialog::on_btn_FindAll_clicked()
{
    if (!mSearch.isSearching()) {
        if (ui->combo_search->currentText().isEmpty()) return;

        clearResultsView();
        insertHistory();

        mSearch.setParameters(getFilesByScope(), createRegex());
        mShowResults = true;
        mSearch.start();
    } else {
        mSearch.stop();
    }
}

void SearchDialog::intermediateUpdate(int hits)
{
    setSearchStatus(Search::Searching, hits);
}

void SearchDialog::finalUpdate()
{
    updateUi();

    if (mShowResults) {
        if (mSearchResultModel) delete mSearchResultModel;
        mSearchResultModel = new SearchResultModel(createRegex(), mSearch.results());
        emit showResults(mSearchResultModel);
    }

    updateEditHighlighting();
    updateNrMatches();
}

void SearchDialog::updateUi()
{
    bool searching = mSearch.isSearching();

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
    ui->combo_fileExcludePattern->setEnabled(!searching);
    ui->combo_scope->setEnabled(!searching);
    ui->combo_search->setEnabled(!searching);
    ui->txt_replace->setEnabled(!searching);
    ui->label->setEnabled(!searching);
    ui->label_2->setEnabled(!searching);
    ui->label_3->setEnabled(!searching);

    if (!searching) updateComponentAvailability();

    QApplication::processEvents();
}

QSet<FileMeta*> SearchDialog::getFilesByScope(bool ignoreReadOnly)
{
    QSet<FileMeta*> files;
    switch (ui->combo_scope->currentIndex()) {
        case Search::ThisFile: {
            if (mCurrentEditor)
                files << mFileHandler->fileMeta(mCurrentEditor);
            break;
        }
        case Search::ThisProject: {
            PExFileNode* p = mFileHandler->fileNode(mCurrentEditor);
            if (!p) return files;
            for (PExFileNode *c :p->assignedProject()->listFiles()) {
                files.insert(c->file());
            }
            break;
        }
        case Search::Selection: {
            if (mCurrentEditor)
                files << mFileHandler->fileMeta(mCurrentEditor);
            break;
        }
        case Search::OpenTabs: {
            files = mFileHandler->openFiles();
            break;
        }
        case Search::AllFiles: {
            files = mFileHandler->fileMetas();
            break;
        }
        case Search::Folder: {
            if (!mSearch.lastFolder().isEmpty()) {
                QDirIterator it(mSearch.lastFolder(), QDir::Files, QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    QString path = it.next();
                    if (path.isEmpty()) break;

                    files.insert(mFileHandler->findOrCreateFile(path));
                }
            }
        }
        default: break;
    }

    return filterFiles(files, ignoreReadOnly);
}

QSet<FileMeta*> SearchDialog::filterFiles(QSet<FileMeta*> files, bool ignoreReadOnly)
{
    bool ignoreWildcard = selectedScope() == Search::ThisFile || selectedScope() == Search::Selection;

    // create list of include filter regexes
    QStringList includeFilter = ui->combo_filePattern->currentText().split(',', Qt::SkipEmptyParts);
    QList<QRegExp> includeFilterList;
    for (const QString &s : qAsConst(includeFilter))
        includeFilterList.append(QRegExp("*" + s.trimmed(), Qt::CaseInsensitive, QRegExp::Wildcard));

    // create list of exclude filters
    QStringList excludeFilter = ui->combo_fileExcludePattern->currentText().split(',', Qt::SkipEmptyParts);
    QList<QRegExp> excludeFilterList;
    for (const QString &e : qAsConst(excludeFilter))
        excludeFilterList.append(QRegExp("*" + e.trimmed(), Qt::CaseInsensitive, QRegExp::Wildcard));


    // filter files
    QSet<FileMeta*> res;
    for (FileMeta* fm : qAsConst(files)) {
        if (!fm) continue;
        bool include = includeFilterList.count() == 0;

        for (const QRegExp &wildcard : qAsConst(includeFilterList)) {
            include = wildcard.exactMatch(fm->location());
            if (include) break; // one match is enough, dont overwrite result
        }

        if (include)
        for (const QRegExp &wildcard : qAsConst(excludeFilterList)) {
            include = !wildcard.exactMatch(fm->location());
            if (!include) break;
        }

        if ((include || ignoreWildcard) && (!ignoreReadOnly || !fm->isReadOnly()))
            res.insert(fm);
    }
    return res;
}

void SearchDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    if (!mSearch.isSearching()) {
        autofillSearchField();
        updateComponentAvailability();
    }
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
    mSearch.invalidateCache();
}

void SearchDialog::keyPressEvent(QKeyEvent* e)
{
    if ( isVisible() && ((e->key() == Qt::Key_Escape) || (e == Hotkey::SearchOpen)) ) {
        e->accept();
        emit setWidgetPosition(pos());
        hide();
        if (mFileHandler->fileNode(mCurrentEditor)) {
            if (lxiviewer::LxiViewer* lv = ViewHelper::toLxiViewer(mCurrentEditor))
                lv->textView()->setFocus();
            else
                mCurrentEditor->setFocus();
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
        emit openHelpDocument(help::HelpData::getChapterLocation(help::DocumentType::StudioMain),
                                  help::HelpData::getStudioSectionAnchor(
                                      help::HelpData::getStudioSectionName(
                                          help::StudioSection::SearchAndReplace)));
    }
    QDialog::keyPressEvent(e);
}

void SearchDialog::on_combo_scope_currentIndexChanged(int)
{
    searchParameterChanged();
    updateComponentAvailability();
}

void SearchDialog::on_btn_back_clicked()
{
    findNextPrev(true);
}

void SearchDialog::on_btn_forward_clicked()
{
    findNextPrev(false);
}

void SearchDialog::findNextPrev(bool backwards) {
    if (ui->combo_search->currentText().isEmpty()) return;

    mShowResults = false;
    mSearch.setParameters(getFilesByScope(), createRegex(), backwards);

    insertHistory();
    mSearch.findNext(backwards ? Search::Backward : Search::Forward);
}

void SearchDialog::on_btn_clear_clicked()
{
    if (mSearch.hasSearchSelection()) {
        clearSelection();
    } else {
        clearSearch();
    }
    clearSearchSelection();
    mSearch.invalidateCache();
    updateEditHighlighting();
    updateClearButton();
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
    QString file = "";
    if(mCurrentEditor) file = ViewHelper::location(mCurrentEditor);

    // if unknown get cursor from current editor
    if (lineNr == -1 || colNr == -1) {
        AbstractEdit* edit = ViewHelper::toAbstractEdit(mCurrentEditor);
        TextView* tv = ViewHelper::toTextView(mCurrentEditor);
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
            emit selectResult(i);
            updateNrMatches(i + 1);
            return i;
        }
    }
    emit selectResult(-1);
    updateNrMatches();

    return -1;
}

void SearchDialog::on_combo_search_currentTextChanged(const QString)
{
    if (!mSuppressParameterChangedEvent)
        searchParameterChanged();
}

void SearchDialog::searchParameterChanged()
{
    setSearchStatus(Search::Clear);
    mSearch.reset();
}

void SearchDialog::on_cb_caseSens_stateChanged(int)
{
    searchParameterChanged();
}

void SearchDialog::updateComponentAvailability()
{
    // activate search for certain filetypes, unless scope is set to Folder then always activate.
    bool allFiles = selectedScope() == Search::Folder;
    bool activateSearch = allFiles || getFilesByScope().size() > 0 ||
                                        (ViewHelper::editorType(mCurrentEditor) == EditorType::source
                                            || ViewHelper::editorType(mCurrentEditor) == EditorType::txt
                                            || ViewHelper::editorType(mCurrentEditor) == EditorType::lxiLst
                                            || ViewHelper::editorType(mCurrentEditor) == EditorType::txtRo);

    bool replacableFileInScope = getFilesByScope(true).size() > 0;
    bool activateReplace = allFiles || replacableFileInScope;

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

    bool activateFileFilters = activateSearch && !(ui->combo_scope->currentIndex() == Search::ThisFile
                                    || ui->combo_scope->currentIndex() == Search::Selection);
    ui->combo_filePattern->setEnabled(activateFileFilters);
    ui->combo_fileExcludePattern->setEnabled(activateFileFilters);
}

void SearchDialog::updateClearButton()
{
    if (mSearch.hasSearchSelection())
        ui->btn_clear->setText("Clear Selection");
    else ui->btn_clear->setText("Clear");
}

void SearchDialog::clearSelection()
{
    if (AbstractEdit* ae = ViewHelper::toAbstractEdit(mCurrentEditor)) {
        QTextCursor tc = ae->textCursor();
        tc.clearSelection();
        ae->setTextCursor(tc);
    } else if (TextView* tv = ViewHelper::toTextView(mCurrentEditor)) {
        QTextCursor tc = tv->edit()->textCursor();
        tc.clearSelection();
        tv->edit()->setTextCursor(tc);
    }
}

void SearchDialog::clearSearchSelection()
{
    if (AbstractEdit* ae = ViewHelper::toAbstractEdit(mCurrentEditor))
        ae->clearSearchSelection();
    else if (TextView* tv = ViewHelper::toTextView(mCurrentEditor))
        tv->clearSearchSelection();
}

void SearchDialog::clearSearch()
{
    mSuppressParameterChangedEvent = true;
    ui->combo_search->clearEditText();
    ui->txt_replace->clear();
    mSuppressParameterChangedEvent = false;
    mSearch.reset();
    mSearch.setParameters(QSet<FileMeta*>(), QRegularExpression(""));

    clearSelection();

    clearResultsView();
    updateEditHighlighting();
}

void SearchDialog::updateEditHighlighting()
{
    if (!mCurrentEditor) return;

    if (AbstractEdit* ae = ViewHelper::toCodeEdit(mCurrentEditor))
        ae->updateExtraSelections();
    else if (TextView* tv = ViewHelper::toTextView(mCurrentEditor))
        tv->updateExtraSelections();
    emit extraSelectionsUpdated();
}

void SearchDialog::clearResultsView()
{
    setSearchStatus(Search::Clear);
    emit closeResults();
}

void SearchDialog::setSearchStatus(Search::Status status, int hits)
{
    QString searching = "Searching (";
    QString dotAnim = ".";

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
        if (selectedScope() == Search::Scope::Selection && !mSearch.hasSearchSelection())
            ui->lbl_nrResults->setText("Selection missing.");
        else
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

    int linebreak = searchText.indexOf("\n");
    searchText = searchText.left(linebreak);

    addEntryToComboBox(ui->combo_search);
    addEntryToComboBox(ui->combo_filePattern);
    addEntryToComboBox(ui->combo_fileExcludePattern);
    addEntryToComboBox(ui->combo_path);
}

void SearchDialog::addEntryToComboBox(QComboBox* box)
{
    QString content = box->currentText();
    if (content.isEmpty()) return;

    if (box->findText(content) == -1) {
        box->insertItem(0, content);
    } else {
        mSuppressParameterChangedEvent = true;
        box->removeItem(box->findText(content));
        box->insertItem(0, content);
        box->setCurrentIndex(0);
        mSuppressParameterChangedEvent = false;
    }
}

void SearchDialog::autofillSearchField()
{
    QWidget *widget = mCurrentEditor;
    PExAbstractNode *fsc = mFileHandler->fileNode(widget);
    if (!fsc) return;

    QString searchText;
    if (CodeEdit *ce = ViewHelper::toCodeEdit(widget)) {
        if (ce->textCursor().hasSelection())
            searchText = ce->textCursor().selection().toPlainText();
        else searchText = ce->wordUnderCursor();
    } else if (TextView *tv = ViewHelper::toTextView(widget)) {
        if (tv->hasSelection())
            searchText = tv->selectedText();
        else searchText = tv->wordUnderCursor();
    }

    if (searchText.isEmpty()) searchText = ui->combo_search->itemText(0);

    searchText = searchText.split("\n").at(0);
    ui->combo_search->setEditText(searchText);
    ui->combo_search->setFocus();
    ui->combo_search->lineEdit()->selectAll();
}

void SearchDialog::updateNrMatches(int current)
{
    int size = qMin(MAX_SEARCH_RESULTS, mSearch.results().size());
    ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);

    if (current == 0) {
        if (size == 0) setSearchStatus(Search::NoResults);
        else ui->lbl_nrResults->setText(QString::number(size) + ((size == 1) ? " match" : " matches"));

        if (size >= MAX_SEARCH_RESULTS) {
            ui->lbl_nrResults->setText( QString::number(MAX_SEARCH_RESULTS) + "+ matches");
            ui->lbl_nrResults->setToolTip("Search is limited to "
                                              + QString::number(MAX_SEARCH_RESULTS) + " matches.");
        } else {
            ui->lbl_nrResults->setToolTip("");
        }
    } else {
        ui->lbl_nrResults->setText(QString::number(current) + " / "
                                     + QString::number(size) + ((size >= MAX_SEARCH_RESULTS) ? "+" : ""));
    }
}

QRegularExpression SearchDialog::createRegex()
{
    QString searchTerm = ui->combo_search->currentText();
    QRegularExpression searchRegex(searchTerm);
    searchRegex.setPatternOptions(QRegularExpression::MultilineOption);

    if (!regex()) searchRegex.setPattern(QRegularExpression::escape(searchTerm));
    if (wholeWords()) searchRegex.setPattern("(?<!\\w)" + searchRegex.pattern() + "(?!\\w)");
    if (!caseSens()) searchRegex.setPatternOptions(searchRegex.patternOptions() | QRegularExpression::CaseInsensitiveOption);

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

Search::Scope SearchDialog::selectedScope()
{
    return (Search::Scope) ui->combo_scope->currentIndex();
}

void SearchDialog::setSelectedScope(int index)
{
    ui->combo_scope->setCurrentIndex(index);
}

Search* SearchDialog::search()
{
    return &mSearch;
}

AbstractSearchFileHandler *SearchDialog::fileHandler()
{
    return mFileHandler;
}

QWidget *SearchDialog::currentEditor()
{
    return mCurrentEditor;
}

}
}
}
