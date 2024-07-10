/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include <QRegularExpression>
#include "search/fileworker.h"
#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "settings.h"
#include "syntax.h"
#include "exception.h"
#include "searchresultmodel.h"
#include "viewhelper.h"
#include "lxiviewer/lxiviewer.h"
#include "../keys.h"
#include "help/helpdata.h"
#include "editors/sysloglocator.h"

namespace gams {
namespace studio {
namespace search {

SearchDialog::SearchDialog(AbstractSearchFileHandler* fileHandler, MainWindow* parent) :
    QDialog(parent), mFileWorker(fileHandler), ui(new Ui::SearchDialog), mMain(parent),
    mFileHandler(fileHandler), mSearch(this, fileHandler)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);
    ui->frame_findinfiles->setVisible(false);
    ui->frame_filters->setVisible(false);
    adjustHeight();

    restoreSettings();
    connect(&mSearch, &Search::updateUI, this, &SearchDialog::updateDialogState);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    emit toggle();
    move(mLastPosition);

    if (!mSearch.isSearching())
        updateDialogState();
}

void SearchDialog::closeEvent(QCloseEvent *event)
{
    mSearch.requestStop();
    event->ignore();
    QDialog::closeEvent(event);
}

void SearchDialog::moveEvent(QMoveEvent *event)
{
    QDialog::moveEvent(event);
    mLastPosition = event->pos();
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
    if (mCurrentEditor != editor) {
        mCurrentEditor = editor;
        mSearch.activeFileChanged();
    }
}

void SearchDialog::on_btn_Replace_clicked()
{
    if (!checkSearchTerm()) return;
    insertHistory();

    mSearch.start(createSearchParameters(false, true, false));
    mSearch.replaceNext(ui->txt_replace->text());
}

SearchParameters SearchDialog::createSearchParameters(bool showResults, bool ignoreReadonly, bool searchBackwards)
{
    SearchParameters parameters;
    parameters.regex = createRegex();
    parameters.searchTerm = searchTerm();
    parameters.replaceTerm = ui->txt_replace->text();

    parameters.useRegex = regex();
    parameters.caseSensitive = caseSens();
    parameters.searchBackwards = searchBackwards;
    parameters.showResults = showResults;
    parameters.ignoreReadOnly = ignoreReadonly;

    parameters.currentFile = mFileHandler->fileMeta(mCurrentEditor);

    parameters.scope = selectedScope();
    parameters.path = ui->combo_path->currentText();
    parameters.excludeFilter = ui->combo_fileExcludePattern->currentText().split(',', Qt::SkipEmptyParts);
    parameters.includeFilter = ui->combo_filePattern->currentText().split(',', Qt::SkipEmptyParts);
    parameters.includeSubdirs = ui->cb_subdirs->isChecked();

    return parameters;
}

void SearchDialog::on_btn_ReplaceAll_clicked()
{
    if (!checkSearchTerm()) return;

    clearResultsView();
    insertHistory();

    mSearch.replaceAll(createSearchParameters(false, true));
}

void SearchDialog::on_btn_FindAll_clicked()
{
    if (!mSearch.isSearching()) {
        if (!checkSearchTerm()) return;

        clearResultsView();
        insertHistory();

        mSearch.start(createSearchParameters(true, false, false));
    } else {
        mSearch.requestStop();
    }
}

void SearchDialog::intermediateUpdate(int hits)
{
    setSearchStatus(Search::Searching, hits);
}

void SearchDialog::finalUpdate()
{
    setSearchStatus(Search::Ok, mSearch.results().size());
    updateDialogState();
    updateEditHighlighting();
}

void SearchDialog::relaySearchResults(bool showResults, QList<Result> *results) {
    if ((showResults && results->size() > 0) || (mMain->resultsView() && mMain->resultsView()->isVisible())) {
        if (mSearchResultModel) delete mSearchResultModel;
        mSearchResultModel = new SearchResultModel(createRegex(), mSearch.results());
        emit updateResults(mSearchResultModel);
    }
}

void SearchDialog::updateDialogState()
{
    bool searching = mSearch.isSearching();

    if (searching)
        ui->btn_FindAll->setText("Abort");
    else ui->btn_FindAll->setText("Find All");
    updateClearButton();

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
    ui->label_4->setEnabled(!searching);
    ui->combo_path->setEnabled(!searching);
    ui->cb_subdirs->setEnabled(!searching);
    ui->btn_browse->setEnabled(!searching);
    ui->label_5->setEnabled(!searching);

    if (!searching) updateComponentAvailability();

    repaint();
}

QList<SearchFile> SearchDialog::getFilesByScope(const SearchParameters &parameters)
{
    QList<SearchFile> files;
    mFileWorker.setParameters(parameters);
    switch (ui->combo_scope->currentIndex()) {
        case Scope::ThisFile: {
            if (mCurrentEditor)
                if (FileMeta* fm = mFileHandler->fileMeta(mCurrentEditor))
                    files << SearchFile(fm);
            break;
        }
        case Scope::ThisProject: {
            PExProjectNode* p = mFileHandler->findProject(mCurrentEditor);
            if (!p) return QList<SearchFile>();

            for (PExFileNode *f : p->listFiles())
                files << SearchFile(f->file());
            files = mFileWorker.filterFiles(files, parameters);
            break;
        }
        case Scope::Selection: {
            if (mCurrentEditor)
                if (FileMeta* fm = mFileHandler->fileMeta(mCurrentEditor))
                    files << SearchFile(fm);
            break;
        }
        case Scope::OpenTabs: {
            for (FileMeta* fm : mFileHandler->openFiles())
                files << SearchFile(fm);
            files = mFileWorker.filterFiles(files, parameters);
            break;
        }
        case Scope::AllFiles: {
            for (FileMeta* fm : mFileHandler->fileMetas())
                files << SearchFile(fm);
            files = mFileWorker.filterFiles(files, parameters);
            break;
        }
        case Scope::Folder: {
            files << mFileWorker.collectFilesInFolder();
        }
        default: break;
    }
    return files;
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
        mSearch.requestStop();
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

void SearchDialog::on_combo_scope_currentIndexChanged(int scope)
{
    searchParameterChanged();
    updateDialogState();

    setSearchSelectionActive(scope == Scope::Selection);

    ui->frame_findinfiles->setVisible(ui->combo_scope->currentIndex() == Scope::Folder);
    ui->frame_filters->setVisible(!(ui->combo_scope->currentIndex() == Scope::ThisFile
                                    || ui->combo_scope->currentIndex() == Scope::Selection));
    adjustHeight();

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
    if (!checkSearchTerm()) return;

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
    setSearchSelectionActive(false);
    mSearch.invalidateCache();
}

void SearchDialog::filesChanged()
{
    if (mSearch.scope() == Scope::ThisProject)
        mSearch.invalidateCache();
}

void SearchDialog::on_cb_wholeWords_stateChanged(int)
{
    searchParameterChanged();
}

void SearchDialog::on_cb_regex_stateChanged(int)
{
    searchParameterChanged();
}

void SearchDialog::on_cb_subdirs_stateChanged(int)
{
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
        const Result& match = list.at(i);
        if (file == match.filepath() && match.lineNr() == lineNr && match.colNr() == colNr - match.length()) {
            emit selectResult(i);
            updateMatchLabel(i + 1);
            return i;
        }
    }
    emit selectResult(-1);
    updateMatchLabel();

    return -1;
}

void SearchDialog::checkRegex()
{
    QRegularExpression re(ui->combo_search->currentText());
    if (regex() && !re.isValid())
        setSearchStatus(Search::InvalidRegex);
    updateDialogState();
}

void SearchDialog::on_combo_search_currentTextChanged(const QString&)
{
    searchParameterChanged();
    checkRegex();
}

void SearchDialog::searchParameterChanged()
{
    if (!mSuppressParameterChangedEvent) {
        setSearchStatus(Search::Clear);
        mCurrentSearchGroup = nullptr;
        mSearch.reset();
    }
    checkRegex();
}

void SearchDialog::on_cb_caseSens_stateChanged(int)
{
    searchParameterChanged();
}

void SearchDialog::updateComponentAvailability()
{
    // activate search for certain filetypes, unless scope is set to Folder then always activate.
    bool allFiles = selectedScope() == Scope::Folder;
    bool activateSearch = allFiles || getFilesByScope(createSearchParameters(false)).size() > 0 ||
                                        (ViewHelper::editorType(mCurrentEditor) == EditorType::source
                                            || ViewHelper::editorType(mCurrentEditor) == EditorType::txt
                                            || ViewHelper::editorType(mCurrentEditor) == EditorType::lxiLst
                                            || ViewHelper::editorType(mCurrentEditor) == EditorType::txtRo);

    QRegularExpression re(ui->combo_search->currentText());
    bool validRegex = !regex() || re.isValid();

    bool activateSearchButtons = activateSearch && validRegex;
    bool replacableFileInScope = allFiles || getFilesByScope(createSearchParameters(false, true)).size() > 0;
    bool activateReplace = (allFiles || replacableFileInScope) && validRegex;

    // replace actions (!readonly):
    ui->txt_replace->setEnabled(activateReplace);
    ui->btn_Replace->setEnabled(activateReplace);
    ui->btn_ReplaceAll->setEnabled(activateReplace);

    // search actions (!gdx || !lst):
    ui->combo_search->setEnabled(activateSearch);
    ui->btn_FindAll->setEnabled(activateSearchButtons);
    ui->btn_forward->setEnabled(activateSearchButtons);
    ui->btn_back->setEnabled(activateSearchButtons);
    ui->btn_clear->setEnabled(activateSearch);

    ui->cb_caseSens->setEnabled(activateSearch);
    ui->cb_regex->setEnabled(activateSearch);
    ui->cb_wholeWords->setEnabled(activateSearch);

    bool activateFileFilters = activateSearch && !(ui->combo_scope->currentIndex() == Scope::ThisFile
                                    || ui->combo_scope->currentIndex() == Scope::Selection);
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
        ae->clearSearchSelection();
    } else if (TextView* tv = ViewHelper::toTextView(mCurrentEditor)) {
        tv->clearSelection();
        tv->clearSearchSelection();
    }
    updateClearButton();
}

void SearchDialog::setSearchSelectionActive(bool active)
{
    if (AbstractEdit* ae = ViewHelper::toAbstractEdit(mCurrentEditor))
        ae->setSearchSelectionActive(active);
    else if (TextView* tv = ViewHelper::toTextView(mCurrentEditor))
        tv->setSearchSelectionActive(active);

    updateEditHighlighting();
    updateClearButton();
}

void SearchDialog::clearSearch()
{
    mSuppressParameterChangedEvent = true;
    ui->combo_search->clearEditText();
    ui->txt_replace->clear();
    mSuppressParameterChangedEvent = false;
    mSearch.reset();

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
    if (status == Search::Ok && mSearchStatus == Search::InvalidPath)
        return;
    else mSearchStatus = status;

    QString dotAnim = ".";
    QString inXFiles = (mFilesInScope > 1 ? (" in " + QString::number(mFilesInScope) + " files") : "");

    hits = (hits > MAX_SEARCH_RESULTS-1) ? MAX_SEARCH_RESULTS : hits;

    if (status == Search::Ok && hits == 0)
        status = Search::NoResults;

    switch (status) {
    case Search::Searching:
        ui->lbl_nrResults->setText("Searching" + inXFiles
                                   + dotAnim.repeated(mSearchAnimation++ % 4)
                                   + " (Results: " + QString::number(hits) + ")"
                                    );
        break;
    case Search::Ok:
        ui->lbl_nrResults->setText(QString::number(hits) + ((hits == 1)
                                    ? " match" : " matches") + inXFiles + ".");
        break;
    case Search::NoResults:
        if (selectedScope() == Scope::Selection && !mSearch.hasSearchSelection())
            ui->lbl_nrResults->setText("Selection missing.");
        else {
            if (mFilesInScope == 1)
                ui->lbl_nrResults->setText("No results in 1 file.");
            else
                ui->lbl_nrResults->setText("No results" + inXFiles);
        }
        break;
    case Search::Clear:
        ui->lbl_nrResults->setText("");
        break;
    case Search::Replacing:
        ui->lbl_nrResults->setText("Replacing... " + inXFiles);
        break;
    case Search::InvalidPath:
        ui->lbl_nrResults->setText("Invalid Path: \"" + ui->combo_path->currentText() + "\"");
        break;
    case Search::EmptySearchTerm:
        ui->lbl_nrResults->setText("Please enter search term.");
        break;
    case Search::InvalidRegex:
        ui->lbl_nrResults->setText("Invalid Regular Expression: " + createRegex().errorString());
        break;
    case Search::CollectingFiles:
        ui->lbl_nrResults->setText("Collecting files...");
        break;
    }
    repaint();
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

void SearchDialog::autofillSearchDialog()
{
    if (mSearch.isSearching()) return;

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

    if (ui->combo_path->count() == 0)
        ui->combo_path->setEditText(fsc->assignedProject()->workDir());
}

void SearchDialog::updateMatchLabel(int current, int max)
{
    int size = max != -1 ? max : qMin(MAX_SEARCH_RESULTS, mSearch.results().size());
    QString files = (mFilesInScope > 1 ? " in " + QString::number(mFilesInScope) + " files." : "");

    if (current == 0) {
        if (size >= MAX_SEARCH_RESULTS) {
            ui->lbl_nrResults->setText(QString::number(MAX_SEARCH_RESULTS) + "+ matches" + files + ".");
            ui->lbl_nrResults->setToolTip("Search is limited to "
                                              + QString::number(MAX_SEARCH_RESULTS) + " matches.");
        } else {
            ui->lbl_nrResults->setToolTip("");
        }
    } else {
        ui->lbl_nrResults->setText(QString::number(current) + " / "
                                     + QString::number(size) + ((size >= MAX_SEARCH_RESULTS) ? "+" : "") + files);
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

QString SearchDialog::searchTerm()
{
    return ui->combo_search->currentText();
}

Scope SearchDialog::selectedScope()
{
    return (Scope) ui->combo_scope->currentIndex();
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

void SearchDialog::on_btn_browse_clicked()
{
    QString oldPath = ui->combo_path->currentText();

    QDir openPath(".");
    if (!oldPath.isEmpty()) {
        openPath = QDir(oldPath);
    } else if (FileMeta* fm = mFileHandler->fileMeta(mCurrentEditor)) {
        openPath = QDir(QFileInfo(fm->location()).absolutePath());
    }

    QString path = QFileDialog::getExistingDirectory(this, "Pick a folder to search", openPath.path());
    if (!path.isEmpty())
        ui->combo_path->setCurrentText(path);
}

void SearchDialog::adjustHeight()
{
    // set minimum possible height
    QApplication::processEvents(); // this is necessary for correct resizing of the dialog
    resize(width(), 1);
}

bool SearchDialog::checkSearchTerm()
{
    if (ui->combo_search->currentText().isEmpty()) {
        setSearchStatus(Search::EmptySearchTerm);
        return false;
    }
    return true;
}

void SearchDialog::jumpToResult(int index) {
    if (!mSearch.hasCache() || !(index > -1 && index < mSearch.results().size())) return;

    Result r = mSearch.results().at(index);
    jumpToResult(r);
}

void SearchDialog::jumpToResult(Result r)
{
    if (!QFileInfo::exists(r.filepath())) {
        SysLogLocator::systemLog()->append("File not found: " + r.filepath(), LogMsgType::Error);
        return;
    }

    PExFileNode* fn = mFileHandler->findFileNode(r.filepath());
    if (!r.parentGroup().isValid()) {
        if (fn) r.setParentGroup(fn->parentNode()->id());
    }

    // create group for search results
    if (!r.parentGroup().isValid() && !Settings::settings()->toBool(skOpenInCurrent)) {
        QString name = "Search: " + ui->combo_search->currentText();

        // find existing search group
        QVector<PExProjectNode*> projects = mFileHandler->projects();
        for (PExGroupNode* g : std::as_const(projects)) {
            if (g->name() == name) {
                mCurrentSearchGroup = g->toProject();
                break;
            } else mCurrentSearchGroup = nullptr;
        }
        if (!mCurrentSearchGroup) {
            QFileInfo dir(r.filepath());
            mCurrentSearchGroup = mFileHandler->createProject(name, dir.absolutePath());
        }
    }

    if (!fn) {
        fn = mFileHandler->openFile(r.filepath(), mCurrentSearchGroup);
        if (!fn) EXCEPT() << "File not found: " << r.filepath();
    }

    NodeId nodeId = (r.parentGroup().isValid()) ? r.parentGroup() : fn->assignedProject()->id();
    fn->file()->jumpTo(nodeId, true, r.lineNr()-1, qMax(r.colNr(), 0), r.length());

}

void SearchDialog::show(QPoint pos)
{
    QDialog::show();
    move(pos);
}

QPoint SearchDialog::lastPosition()
{
    return mLastPosition;
}

bool SearchDialog::hasLastPosition()
{
    return !mLastPosition.isNull();
}

void SearchDialog::setSearchedFiles(int files)
{
    mFilesInScope = files;
}

void SearchDialog::on_combo_path_currentTextChanged(const QString&)
{
    searchParameterChanged();
}

void SearchDialog::on_combo_fileExcludePattern_currentTextChanged(const QString&)
{
    searchParameterChanged();
}

}
}
}
