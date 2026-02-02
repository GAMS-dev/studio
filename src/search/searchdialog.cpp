/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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

#include "fileworker.h"
#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "abstractsearchfilehandler.h"
#include "settings.h"
#include "syntax.h"
#include "exception.h"
#include "searchresultmodel.h"
#include "viewhelper.h"
#include "lxiviewer/lxiviewer.h"
#include "../keys.h"
#include "help/helpdata.h"
#include "editors/sysloglocator.h"

const int CComboBoxListLimit = 20;

namespace gams {
namespace studio {
namespace search {

SearchDialog::SearchDialog(AbstractSearchFileHandler* fileHandler, MainWindow* parent)
    : QDialog(parent)
    , ui(new Ui::SearchDialog)
    , mFileWorker(new FileWorker(fileHandler))
    , mMain(parent)
    , mFileHandler(fileHandler)
    , mSearch(this, fileHandler)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    ui->frame_findinfiles->setVisible(false);
    ui->frame_filters->setVisible(false);
    adjustHeight();
    setupConnections();
    restoreSettings();
    connect(&mSearch, &Search::updateUI, this, &SearchDialog::updateDialogState);
    connect(this, &QDialog::rejected, this, [this]{
        if (selectedScope() == Scope::Selection || selectedScope() == Scope::ThisFile) {
            ResultsView *view = mMain ? mMain->resultsView() : nullptr;
            if (view && view->isVisible())
                return;
            mSearch.reset();
            clearSelection();
            clearResultsView();
            updateEditHighlighting();
        }
    });
    ui->termComboBox->installEventFilter(this);
    ui->combo_excludePattern->installEventFilter(this);
    ui->combo_includePattern->installEventFilter(this);
    ui->directoryComboBox->installEventFilter(this);
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

void SearchDialog::setupConnections()
{
    connect(ui->btn_FindAll, &QPushButton::clicked,
            this, &SearchDialog::findAll);
    connect(ui->btn_Replace, &QPushButton::clicked,
            this, &SearchDialog::replace);
    connect(ui->btn_ReplaceAll, &QPushButton::clicked,
            this, &SearchDialog::replaceAll);
    connect(ui->btn_browse, &QPushButton::clicked,
            this, &SearchDialog::browse);
    connect(ui->scopeComboBox, &QComboBox::currentIndexChanged,
            this, &SearchDialog::changeScope);
    connect(ui->termComboBox, &QComboBox::currentTextChanged, this, [this]{
        searchParameterChanged();
        checkRegex();
    });
    connect(&mSearch, &Search::updateUI,
            this, &SearchDialog::updateDialogState);
    connect(ui->btn_backward, &QPushButton::clicked,
            this, [this]{ findNextPrev(true); });
    connect(ui->btn_forward, &QPushButton::clicked,
            this, [this]{ findNextPrev(false); });
    connect(ui->cb_caseSens, &QCheckBox::checkStateChanged,
            this, &SearchDialog::searchParameterChanged);
    connect(ui->cb_wholeWords, &QCheckBox::checkStateChanged,
            this, &SearchDialog::searchParameterChanged);
    connect(ui->cb_regex, &QCheckBox::checkStateChanged,
            this, &SearchDialog::searchParameterChanged);
    connect(ui->cb_subdirs, &QCheckBox::checkStateChanged,
            this, &SearchDialog::searchParameterChanged);
    connect(ui->directoryComboBox, &QComboBox::currentIndexChanged,
            this, &SearchDialog::searchParameterChanged);
    connect(ui->combo_excludePattern, &QComboBox::currentIndexChanged,
            this, &SearchDialog::searchParameterChanged);
}

void SearchDialog::restoreSettings()
{
    Settings *settings = Settings::settings();
    auto terms = settings->toString(skSearchTermList).split(Parameters::listSeparator(), Qt::SkipEmptyParts);
    ui->termComboBox->addItems(terms);
    ui->termComboBox->setCurrentText(settings->toString(skSearchTerm));
    ui->replaceEdit->setText(settings->toString(skSearchReplace));
    ui->cb_regex->setChecked(settings->toBool(skSearchUseRegex));
    ui->cb_caseSens->setChecked(settings->toBool(skSearchCaseSens));
    ui->cb_wholeWords->setChecked(settings->toBool(skSearchWholeWords));
    ui->scopeComboBox->setCurrentIndex(settings->toInt(skSearchScope));
    auto inc = settings->toString(skSearchIncludeFilterList).split(Parameters::listSeparator(), Qt::SkipEmptyParts);
    ui->combo_includePattern->addItems(inc);
    ui->combo_includePattern->setCurrentText(settings->toString(skSearchIncludeFilter));
    auto exc = settings->toString(skSearchExcludeFilterList).split(Parameters::listSeparator(), Qt::SkipEmptyParts);
    ui->combo_excludePattern->addItems(exc);
    ui->combo_excludePattern->setCurrentText(settings->toString(skSearchExcludeFilter));
    auto dirs = settings->toString(skSearchDirectoryList).split(Parameters::listSeparator(), Qt::SkipEmptyParts);
    ui->directoryComboBox->addItems(dirs);
    ui->directoryComboBox->setCurrentText(settings->toString(skSearchDirectory));
    ui->cb_subdirs->setChecked(settings->toBool(skSearchIncludeSubdirs));
    ui->lbl_nrResults->setText("");
    ui->termComboBox->setCompleter(nullptr);
}

void SearchDialog::editorChanged(QWidget* editor) {
    if (mCurrentEditor != editor) {
        mCurrentEditor = editor;
        mSearch.activeFileChanged();
    }
}

void SearchDialog::replace()
{
    if (!checkSearchTerm())
        return;
    insertHistory();
    mSearch.start(createSearchParameters(false, true, false));
    mSearch.replaceNext(ui->replaceEdit->text());
}

Parameters SearchDialog::createSearchParameters(bool showResults, bool ignoreReadonly, bool searchBackwards)
{
    Parameters parameters;
    parameters.setRegex(createRegex());
    parameters.setSearchTerm(ui->termComboBox->currentText());
    parameters.setReplaceTerm(ui->replaceEdit->text());

    parameters.setUseRegex(ui->cb_regex->isChecked());
    parameters.setCaseSensitive(ui->cb_caseSens->isChecked());
    parameters.setSearchBackwards(searchBackwards);
    parameters.setShowResults(showResults);
    parameters.setIgnoreReadOnly(ignoreReadonly);

    parameters.setScope(selectedScope());
    parameters.setDirectory(ui->directoryComboBox->currentText());
    parameters.setExcludeFilter(ui->combo_excludePattern->currentText().split(',', Qt::SkipEmptyParts));
    parameters.setIncludeFilter(ui->combo_includePattern->currentText().split(',', Qt::SkipEmptyParts));
    parameters.setIncludeSubdirs(ui->cb_subdirs->isChecked());

    return parameters;
}

void SearchDialog::replaceAll()
{
    if (!checkSearchTerm())
        return;
    clearResultsView();
    insertHistory();
    mSearch.replaceAll(createSearchParameters(false, true));
}

void SearchDialog::findAll()
{
    if (!mSearch.isSearching()) {
        if (!checkSearchTerm())
            return;
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

void SearchDialog::relaySearchResults(bool showResults, QList<Result> *results)
{
    if ((showResults && !results->isEmpty()) || (mMain->resultsView() && mMain->resultsView()->isVisible())) {
        if (mSearchResultModel)
            delete mSearchResultModel;
        mSearchResultModel = new SearchResultModel(createRegex(), mSearch.results());
        emit updateResults(mSearchResultModel);
    }
}

void SearchDialog::updateDialogState()
{
    bool searching = mSearch.isSearching();
    if (searching)
        ui->btn_FindAll->setText("Abort");
    else
        ui->btn_FindAll->setText("Find All");
    updateClearButton();

    // deactivate actions while search is ongoing
    ui->btn_Replace->setEnabled(!searching);
    ui->btn_ReplaceAll->setEnabled(!searching);
    ui->btn_clear->setEnabled(!searching);
    ui->cb_regex->setEnabled(!searching);
    ui->cb_caseSens->setEnabled(!searching);
    ui->cb_wholeWords->setEnabled(!searching);
    ui->combo_includePattern->setEnabled(!searching);
    ui->combo_excludePattern->setEnabled(!searching);
    ui->scopeComboBox->setEnabled(!searching);
    ui->termComboBox->setEnabled(!searching);
    ui->replaceEdit->setEnabled(!searching);
    ui->label->setEnabled(!searching);
    ui->label_2->setEnabled(!searching);
    ui->label_3->setEnabled(!searching);
    ui->label_4->setEnabled(!searching);
    ui->directoryComboBox->setEnabled(!searching);
    ui->cb_subdirs->setEnabled(!searching);
    ui->btn_browse->setEnabled(!searching);
    ui->label_5->setEnabled(!searching);

    if (!searching)
        updateComponentAvailability();

    repaint();
}

QList<SearchFile> SearchDialog::getFilesByScope(const Parameters &parameters)
{
    QList<SearchFile> files, matched;
    mFileWorker->setParameters(parameters);
    switch (ui->scopeComboBox->currentIndex()) {
        case Scope::ThisFile: {
            if (mCurrentEditor) {
                if (FileMeta* fm = mFileHandler->fileMeta(mCurrentEditor))
                    matched << SearchFile(fm);
            }
            break;
        }
        case Scope::ThisProject: {
            PExProjectNode* p = mFileHandler->findProject(mCurrentEditor);
            if (!p)
                return QList<SearchFile>();
            for (PExFileNode *f : p->listFiles())
                files << SearchFile(f->file());
            mFileWorker->filterFiles(files, parameters, matched);
            break;
        }
        case Scope::Selection: {
            if (mCurrentEditor) {
                if (FileMeta* fm = mFileHandler->fileMeta(mCurrentEditor))
                    matched << SearchFile(fm);
            }
            break;
        }
        case Scope::OpenTabs: {
            auto metas =  mFileHandler->openFiles();
            for (FileMeta* fm : std::as_const(metas)) {
                files << SearchFile(fm);
            }
            mFileWorker->filterFiles(files, parameters, matched);
            break;
        }
        case Scope::AllFiles: {
            auto metas = mFileHandler->fileMetas();
            for (FileMeta* fm : std::as_const(metas)) {
                files << SearchFile(fm);
            }
            mFileWorker->filterFiles(files, parameters, matched);
            break;
        }
        case Scope::Folder: {
            matched << mFileWorker->collectFilesInFolder();
        }
        default: break;
    }
    return matched;
}

void SearchDialog::on_searchNext()
{
    emit ui->btn_forward->clicked();
}

void SearchDialog::on_searchPrev()
{
    emit ui->btn_backward->clicked();
}

void SearchDialog::on_documentContentChanged(int from, int charsRemoved, int charsAdded)
{
    Q_UNUSED(from);
    Q_UNUSED(charsRemoved);
    Q_UNUSED(charsAdded);
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
        emit ui->btn_backward->clicked();
        e->accept();
    } else if (e == Hotkey::SearchFindNext) {
        emit ui->btn_forward->clicked();
        e->accept();
    } else if (e->modifiers() & Qt::ShiftModifier && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)) {
        e->accept();
        findAll();
    } else if (e == Hotkey::OpenHelp) {
        emit openHelpDocument(help::HelpData::getChapterLocation(help::DocumentType::StudioMain),
                                  help::HelpData::getStudioSectionAnchor(
                                      help::HelpData::getStudioSectionName(
                                          help::StudioSection::SearchAndReplace)));
    }
    QDialog::keyPressEvent(e);
}

void SearchDialog::changeScope(int scope)
{
    searchParameterChanged();
    updateDialogState();

    setSearchSelectionActive(scope == Scope::Selection);

    ui->frame_findinfiles->setVisible(ui->scopeComboBox->currentIndex() == Scope::Folder);
    ui->frame_filters->setVisible(!(ui->scopeComboBox->currentIndex() == Scope::ThisFile
                                    || ui->scopeComboBox->currentIndex() == Scope::Selection));
    adjustHeight();

}

void SearchDialog::findNextPrev(bool backwards) {
    if (!checkSearchTerm())
        return;
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
    if (mSearch.parameters().scope() == Scope::ThisProject)
        mSearch.invalidateCache();
}

///
/// \brief SearchDialog::updateFindNextLabel calculates match number,
///        updates label and result lists from cursor position
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
        if (file == match.filePath() && match.lineNr() == lineNr && match.colNr() == colNr - match.length()) {
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
    QRegularExpression re(ui->termComboBox->currentText());
    if (ui->cb_regex->isChecked() && !re.isValid())
        setSearchStatus(Search::InvalidRegex);
    updateDialogState();
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

void SearchDialog::updateComponentAvailability()
{
    // activate search for certain filetypes, unless scope is set to Folder then always activate.
    bool allFiles = selectedScope() == Scope::Folder;
    bool activateSearch = allFiles || getFilesByScope(createSearchParameters(false)).size() > 0 ||
                                        (ViewHelper::editorType(mCurrentEditor) == EditorType::source
                                            || ViewHelper::editorType(mCurrentEditor) == EditorType::txt
                                            || ViewHelper::editorType(mCurrentEditor) == EditorType::lxiLst
                                            || ViewHelper::editorType(mCurrentEditor) == EditorType::txtRo);

    QRegularExpression re(ui->termComboBox->currentText());
    bool validRegex = !ui->cb_regex->isChecked() || re.isValid();

    bool activateSearchButtons = activateSearch && validRegex;
    bool replacableFileInScope = allFiles || getFilesByScope(createSearchParameters(false, true)).size() > 0;
    bool activateReplace = (allFiles || replacableFileInScope) && validRegex;

    // replace actions (!readonly):
    ui->replaceEdit->setEnabled(activateReplace);
    ui->btn_Replace->setEnabled(activateReplace);
    ui->btn_ReplaceAll->setEnabled(activateReplace);

    // search actions (!gdx || !lst):
    ui->termComboBox->setEnabled(activateSearch);
    ui->btn_FindAll->setEnabled(activateSearchButtons);
    ui->btn_forward->setEnabled(activateSearchButtons);
    ui->btn_backward->setEnabled(activateSearchButtons);
    ui->btn_clear->setEnabled(activateSearch);

    ui->cb_caseSens->setEnabled(activateSearch);
    ui->cb_regex->setEnabled(activateSearch);
    ui->cb_wholeWords->setEnabled(activateSearch);

    bool activateFileFilters = activateSearch && !(ui->scopeComboBox->currentIndex() == Scope::ThisFile
                                    || ui->scopeComboBox->currentIndex() == Scope::Selection);
    ui->combo_includePattern->setEnabled(activateFileFilters);
    ui->combo_excludePattern->setEnabled(activateFileFilters);
}

void SearchDialog::updateClearButton()
{
    if (mSearch.hasSearchSelection())
        ui->btn_clear->setText("Clear Selection");
    else
        ui->btn_clear->setText("Clear");
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
    ui->termComboBox->clearEditText();
    ui->replaceEdit->clear();
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
    else
        mSearchStatus = status;

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
        ui->lbl_nrResults->setText("Invalid Path: \"" + ui->directoryComboBox->currentText() + "\"");
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
    if (ui->termComboBox->currentText().isEmpty())
        return;
    addEntryToComboBox(ui->termComboBox);
    addEntryToComboBox(ui->combo_includePattern);
    addEntryToComboBox(ui->combo_excludePattern);
    addEntryToComboBox(ui->directoryComboBox);
}

void SearchDialog::addEntryToComboBox(QComboBox* box)
{
    QString content = box->currentText();
    if (content.isEmpty())
        return;
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
    if (mSearch.isSearching())
        return;

    QWidget *widget = mCurrentEditor;
    PExAbstractNode *fsc = mFileHandler->fileNode(widget);
    if (!fsc)
        return;

    QString searchText;
    if (CodeEdit *ce = ViewHelper::toCodeEdit(widget)) {
        if (ce->textCursor().hasSelection())
            searchText = ce->textCursor().selection().toPlainText();
        else
            searchText = ce->wordUnderCursor();
    } else if (TextView *tv = ViewHelper::toTextView(widget)) {
        if (tv->hasSelection())
            searchText = tv->selectedText();
        else
            searchText = tv->wordUnderCursor();
    }

    searchText = searchText.split("\n").at(0);
    ui->termComboBox->setEditText(searchText);
    ui->termComboBox->setFocus();
    ui->termComboBox->lineEdit()->selectAll();

    if (ui->directoryComboBox->count() == 0)
        ui->directoryComboBox->setEditText(fsc->assignedProject()->workDir());
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
    QString searchTerm = ui->termComboBox->currentText();
    QRegularExpression searchRegex(searchTerm);
    searchRegex.setPatternOptions(QRegularExpression::MultilineOption);
    if (!ui->cb_regex->isChecked())
        searchRegex.setPattern(QRegularExpression::escape(searchTerm));
    if (ui->cb_wholeWords->isChecked())
        searchRegex.setPattern("(?<!\\w)" + searchRegex.pattern() + "(?!\\w)");
    if (!ui->cb_caseSens->isChecked())
        searchRegex.setPatternOptions(searchRegex.patternOptions() | QRegularExpression::CaseInsensitiveOption);
    return searchRegex;
}

Scope SearchDialog::selectedScope() const
{
    return (Scope)ui->scopeComboBox->currentIndex();
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

void SearchDialog::browse()
{
    QDir openPath(".");
    QString oldPath = ui->directoryComboBox->currentText();
    if (!oldPath.isEmpty()) {
        openPath = QDir(oldPath);
    } else if (FileMeta* fm = mFileHandler->fileMeta(mCurrentEditor)) {
        openPath = QDir(QFileInfo(fm->location()).absolutePath());
    }
    QString path = QFileDialog::getExistingDirectory(this, "Pick a folder to search", openPath.path());
    if (!path.isEmpty())
        ui->directoryComboBox->setCurrentText(path);
}

void SearchDialog::adjustHeight()
{
    // set minimum possible height
    QApplication::processEvents(); // this is necessary for correct resizing of the dialog
    resize(width(), 1);
}

bool SearchDialog::checkSearchTerm()
{
    if (ui->termComboBox->currentText().isEmpty()) {
        setSearchStatus(Search::EmptySearchTerm);
        return false;
    }
    return true;
}

void SearchDialog::jumpToResult(int index) {
    if (!mSearch.hasCache() || !(index > -1 && index < mSearch.results().size()))
        return;
    jumpToResult(mSearch.results().at(index));
}

void SearchDialog::jumpToResult(Result r)
{
    if (!QFileInfo::exists(r.filePath())) {
        SysLogLocator::systemLog()->append("File not found: " + r.filePath(), LogMsgType::Error);
        return;
    }

    PExFileNode* fn = mFileHandler->findFileNode(r.filePath());
    if (!r.parentGroup().isValid() && fn) {
        r.setParentGroup(fn->parentNode()->id());
    }

    // create group for search results
    if (!r.parentGroup().isValid() && !Settings::settings()->toBool(skOpenInCurrent)) {
        QString name = "-Search: " + ui->termComboBox->currentText();

        // find existing search group
        QVector<PExProjectNode*> projects = mFileHandler->projects();
        for (PExGroupNode* g : std::as_const(projects)) {
            if (g->name() == name) {
                mCurrentSearchGroup = g->toProject();
                break;
            } else {
                mCurrentSearchGroup = nullptr;
            }
        }
        if (!mCurrentSearchGroup) {
            QFileInfo dir(r.filePath());
            mCurrentSearchGroup = mFileHandler->createProject(name, dir.absolutePath());
        }
    }

    if (!fn) {
        fn = mFileHandler->openFile(r.filePath(), mCurrentSearchGroup);
        if (!fn)
            EXCEPT() << "File not found: " << r.filePath();
    }

    NodeId nodeId = (r.parentGroup().isValid()) ? r.parentGroup() : fn->assignedProject()->id();
    fn->file()->jumpTo(nodeId, true, r.lineNr()-1, qMax(r.colNr(), 0), r.length());
}

void SearchDialog::show(const QPoint &pos)
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

void SearchDialog::updateSettings()
{
    auto getItems = [](QComboBox *box) -> QStringList {
        QStringList entries;
        for (int i=0; i < qMin(box->count(), CComboBoxListLimit); ++i) {
            entries.append(box->itemText(i));
        }
        return entries;
    };
    Settings *settings = Settings::settings();
    settings->setString(skSearchTerm, ui->termComboBox->currentText());
    auto terms = getItems(ui->termComboBox).join(Parameters::listSeparator());
    settings->setString(skSearchTermList, terms);
    settings->setString(skSearchReplace, ui->replaceEdit->text());
    settings->setBool(skSearchUseRegex, ui->cb_regex->isChecked());
    settings->setBool(skSearchCaseSens, ui->cb_caseSens->isChecked());
    settings->setBool(skSearchWholeWords, ui->cb_wholeWords->isChecked());
    settings->setInt(skSearchScope, int(selectedScope()));
    settings->setString(skSearchIncludeFilter, ui->combo_includePattern->currentText());
    auto inc = getItems(ui->combo_includePattern).join(Parameters::listSeparator());
    settings->setString(skSearchIncludeFilterList, inc);
    settings->setString(skSearchExcludeFilter, ui->combo_excludePattern->currentText());
    auto exc = getItems(ui->combo_excludePattern).join(Parameters::listSeparator());
    settings->setString(skSearchExcludeFilterList, exc);
    settings->setString(skSearchDirectory, ui->directoryComboBox->currentText());
    auto dirs = getItems(ui->directoryComboBox).join(Parameters::listSeparator());
    settings->setString(skSearchDirectoryList, dirs);
    settings->setBool(skSearchIncludeSubdirs, ui->cb_subdirs->isChecked());
}

bool SearchDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        if (QComboBox *box = qobject_cast<QComboBox*>(watched)) {
            QContextMenuEvent *menuEvent = static_cast<QContextMenuEvent*>(event);
            QMenu *menu = box->lineEdit()->createStandardContextMenu();
            menu->addSeparator();
            menu->addAction("Remove text", this, [box]() {
                for (int i = 0; i < box->count(); ++i) {
                    if (box->itemText(i) == box->currentText()) {
                        box->removeItem(i);
                        if (i < box->count())
                            box->setCurrentIndex(i);
                        else if (box->count() > 0)
                            box->setCurrentIndex(box->count()-1);
                        else
                            box->clearEditText();
                        break;
                    }
                }
            });
            menu->addAction("Clear list", this, [box]() {
                box->clear();
            });
            menu->exec(menuEvent->globalPos());
            delete menu;
            return true;
        }
    }
    return QDialog::eventFilter(watched, event);
}

void SearchDialog::setSearchedFiles(int files)
{
    mFilesInScope = files;
}

}
}
}
