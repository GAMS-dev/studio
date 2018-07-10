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
#include "studiosettings.h"
#include "syntax.h"
#include "exception.h"
#include "searchresultlist.h"

#include <QCompleter>
#include <QMessageBox>
#include <QTextDocumentFragment>

namespace gams {
namespace studio {

SearchDialog::SearchDialog(MainWindow *parent) :
    QDialog(parent), ui(new Ui::SearchDialog), mMain(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    StudioSettings *mSettings = mMain->settings();

    ui->setupUi(this);
    ui->cb_regex->setChecked(mSettings->searchUseRegex());
    ui->cb_caseSens->setChecked(mSettings->searchCaseSens());
    ui->cb_wholeWords->setChecked(mSettings->searchWholeWords());
    ui->combo_scope->setCurrentIndex(mSettings->selectedScopeIndex());
    ui->lbl_nrResults->setText("");
    adjustSize();

    connect(ui->combo_search->lineEdit(), &QLineEdit::returnPressed, this, &SearchDialog::returnPressed);
}

SearchDialog::~SearchDialog()
{
    delete ui;
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

void SearchDialog::on_btn_Replace_clicked()
{
    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(mMain->recent()->editor());
    if (!edit || edit->isReadOnly()) return;

    QString replaceTerm = ui->txt_replace->text();
    if (edit->textCursor().hasSelection())
        edit->textCursor().insertText(replaceTerm);

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
    SearchResultList matches(searchTerm());
    insertHistory();

    setSearchStatus(SearchStatus::Searching);

    switch (ui->combo_scope->currentIndex()) {
    case SearchScope::ThisFile:
        if (mMain->recent()->editor())
            matches.addResultList(findInFile(mMain->projectRepo()->fileNode(mMain->recent()->editor())));
        break;
    case SearchScope::ThisGroup:
        matches.addResultList(findInGroup());
        break;
    case SearchScope::OpenTabs:
        matches.addResultList(findInOpenFiles());
        break;
    case SearchScope::AllFiles:
        matches.addResultList(findInAllFiles());
        break;
    default:
        break;
    }
    updateMatchAmount(matches.size());
    mMain->showResults(matches);
}

QList<Result> SearchDialog::findInAllFiles()
{
    QList<Result> matches;
    ProjectGroupNode *root = mMain->projectRepo()->treeModel()->rootNode();
    ProjectAbstractNode *fsc;
    for (int i = 0; i < root->childCount(); i++) {
        fsc = root->childEntry(i);
        if (fsc->type() == ProjectAbstractNode::ContextType::FileGroup)
            matches.append(findInGroup(fsc));
    }
    return matches;
}

QList<Result> SearchDialog::findInOpenFiles()
{
    QList<Result> matches;
    QWidgetList editList = mMain->projectRepo()->editors();
    ProjectFileNode *fc;
    for (int i = 0; i < editList.size(); i++) {
        fc = mMain->projectRepo()->fileNode(editList.at(i));
        if (fc == nullptr) break;
        matches.append(findInFile(fc));
    }
    return matches;
}

QList<Result> SearchDialog::findInGroup(ProjectAbstractNode *fsc)
{
    QList<Result> matches;

    ProjectGroupNode *fgc = nullptr;
    if (!fsc) {
        ProjectFileNode* fc = mMain->projectRepo()->fileNode(mMain->recent()->editor());
        fgc = (fc ? fc->parentEntry() : nullptr);
        if (!fgc) return QList<Result>();

    } else {
        if (fsc->type() == ProjectGroupNode::FileGroup)
            fgc = static_cast<ProjectGroupNode*>(fsc);
    }

    if (!fgc) return matches;

    for (int i = 0; i < fgc->childCount(); i++) {
        matches.append(findInFile(fgc->childEntry(i)));
    }
    return matches;
}

void SearchDialog::findOnDisk(QRegularExpression searchRegex, bool isOpenFile, ProjectFileNode *fc, SearchResultList* matches)
{
    int lineCounter = 0;
    QFile file(fc->location());
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) { // read file
            lineCounter++;
            QString line = in.readLine();

            QRegularExpressionMatch match;
            QRegularExpressionMatchIterator i = searchRegex.globalMatch(line);
            while (i.hasNext()) {
                match = i.next();
                matches->addResult(lineCounter, match.capturedStart(),
                                  file.fileName(), line.trimmed());
                if (isOpenFile)
                    fc->generateTextMark(TextMark::match, 0, lineCounter-1, match.capturedStart(), match.capturedLength());
            }
        }
        file.close();
    }
}

void SearchDialog::findInDoc(QRegularExpression searchRegex, bool isOpenFile, ProjectFileNode *fc, SearchResultList* matches)
{
    QTextCursor lastItem = QTextCursor(fc->document());
    QTextCursor item;
    do {
        item = fc->document()->find(searchRegex, lastItem, getFlags());
        if (item != lastItem) lastItem = item;
        else break;

        if (!item.isNull()) {
            matches->addResult(item.blockNumber()+1, item.columnNumber() - searchTerm().length(),
                              fc->location(), item.block().text().trimmed());
            if (isOpenFile) {
                int length = item.selectionEnd() - item.selectionStart();
                fc->generateTextMark(TextMark::match, 0, item.blockNumber(),
                                     item.columnNumber() - length, length);
            }
        }
    } while (!item.isNull());

}

QList<Result> SearchDialog::findInFile(ProjectAbstractNode *fsc, bool skipFilters)
{
    if (!fsc) return QList<Result>();

    QRegExp fileFilter(ui->combo_filePattern->currentText().trimmed());
    fileFilter.setPatternSyntax(QRegExp::Wildcard);

    if (!skipFilters) {
        if (((ui->combo_scope->currentIndex() != SearchScope::ThisFile) && (fileFilter.indexIn(fsc->location()) == -1))
                || fsc->location().endsWith("gdx")) {
            return QList<Result>(); // dont search here, return empty
        }
    }

    QString searchTerm = ui->combo_search->currentText();
    SearchResultList matches(searchTerm);
    if (regex()) matches.useRegex(true);

    if (fsc->type() == ProjectAbstractNode::FileGroup) { // or is it a group?
        matches.addResultList(findInGroup(fsc)); // TESTME studio does not support this case as of yet
    } else { // it's a file
        ProjectFileNode *fc(static_cast<ProjectFileNode*>(fsc));
        if (fc == nullptr) FATAL() << "FileNode not found";

        QRegularExpression searchRegex;

        if (!regex()) searchRegex.setPattern(QRegularExpression::escape(searchTerm));
        else searchRegex.setPattern(searchTerm);

        if (wholeWords()) searchRegex.setPattern("\\b" + searchRegex.pattern() + "\\b");
        if (!caseSens()) searchRegex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

        bool isOpenFile = (fc == mMain->projectRepo()->fileNode(mMain->recent()->editor()));

        // when a file has unsaved changes a different search strategy is used.
        if (fc->isModified())
            findInDoc(searchRegex, isOpenFile, fc, &matches);
        else
            findOnDisk(searchRegex, isOpenFile, fc, &matches);

        if (isOpenFile && fc->highlighter())
            fc->highlighter()->rehighlight();
    }
    return matches.resultList();
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

void SearchDialog::simpleReplaceAll()
{
    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(mMain->recent()->editor());
    if (!edit || edit->isReadOnly()) return;

    QString searchTerm = ui->combo_search->currentText();
    QRegularExpression searchRegex(ui->combo_search->currentText());
    QString replaceTerm = ui->txt_replace->text();
    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();

    QList<QTextCursor> hits;
    QTextCursor item;
    QTextCursor lastItem;

    do {
        if (regex())
            item = edit->document()->find(searchRegex, lastItem, searchFlags);
        else
            item = edit->document()->find(searchTerm, lastItem, searchFlags);

        lastItem = item;

        if (!item.isNull())
            hits.append(item);

    } while (!item.isNull());

    QMessageBox msgBox;
    if (hits.length() == 1) {
        msgBox.setText("Replacing 1 occurrence of '" + searchTerm + "' with '" + replaceTerm + "' in file "
                       + mMain->projectRepo()->fileNode(mMain->recent()->editor())->location()
                       + ". Are you sure?");
    } else {
        msgBox.setText("Replacing " + QString::number(hits.length()) + " occurrences of '" +
                       searchTerm + "' with '" + replaceTerm + "' in file "
                       + mMain->projectRepo()->fileNode(mMain->recent()->editor())->location()
                       + ". Are you sure?");
    }
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int answer = msgBox.exec();

    if (answer == QMessageBox::Ok) {
        clearResults();
        edit->textCursor().beginEditBlock();
        foreach (QTextCursor tc, hits) {
            tc.insertText(replaceTerm);
        }
        edit->textCursor().endEditBlock();
    }
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

void SearchDialog::findNext(SearchDirection direction)
{
    if (!mMain->recent()->editor() || ui->combo_search->currentText() == "") return;

    ProjectFileNode *fc = mMain->projectRepo()->fileNode(mMain->recent()->editor());
    if (!fc) return;
    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(mMain->recent()->editor());
    if (!edit) return;

    if (mHasChanged) {
        setSearchStatus(SearchStatus::Searching);
        QApplication::processEvents();
        mCachedResults = findInFile(fc, true);
        mHasChanged = false;
    }

    selectNextMatch(direction, mCachedResults);
}


// this is a workaround for the QLineEdit field swallowing the first enter after a show event
// leading to a search using the last search term instead of the current.
void SearchDialog::returnPressed() {
    if (mFirstReturn) {
        findNext(SearchDialog::Forward);
        mFirstReturn = false;
    }
}

void SearchDialog::autofillSearchField()
{
    QWidget *widget = mMain->recent()->editor();
    AbstractEdit *edit = ProjectFileNode::toAbstractEdit(widget);
    ProjectAbstractNode *fsc = mMain->projectRepo()->fileNode(widget);
    if (!fsc || !edit) return;

    if (edit->textCursor().hasSelection()) {
        ui->combo_search->insertItem(-1, edit->textCursor().selection().toPlainText());
        ui->combo_search->setCurrentIndex(0);
    } else {
        ui->combo_search->setEditText("");
        mFirstReturn = false;
    }

    ui->combo_search->setFocus();
}

void SearchDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    mFirstReturn = true;
    autofillSearchField();
    updateReplaceActionAvailability();
}

void SearchDialog::updateReplaceActionAvailability()
{
    AbstractEdit *edit = ProjectFileNode::toAbstractEdit(mMain->recent()->editor());
    bool isSourceCode = ProjectFileNode::editorType(mMain->recent()->editor()) == ProjectAbstractNode::etSourceCode;

    bool activateSearch = isSourceCode || ProjectFileNode::editorType(mMain->recent()->editor()) == ProjectAbstractNode::etLxiLst;
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
    Q_UNUSED(from); Q_UNUSED(charsRemoved); Q_UNUSED(charsAdded);
    //TODO: make smarter
    invalidateCache();
}

void SearchDialog::invalidateCache()
{
    mHasChanged = true;
}

void SearchDialog::keyPressEvent(QKeyEvent* e)
{
    if ( isVisible() && (e->key() == Qt::Key_Escape) ) {
        e->accept();
        hide();
        if (mMain->projectRepo()->fileNode(mMain->recent()->editor()))
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

void SearchDialog::closeEvent(QCloseEvent *e) {
    setSearchStatus(SearchStatus::Clear);
    QDialog::closeEvent(e);
}

QFlags<QTextDocument::FindFlag> SearchDialog::getFlags()
{
    QFlags<QTextDocument::FindFlag> searchFlags;
    searchFlags.setFlag(QTextDocument::FindCaseSensitively, ui->cb_caseSens->isChecked());
    searchFlags.setFlag(QTextDocument::FindWholeWords, ui->cb_wholeWords->isChecked());

    return searchFlags;
}

int Result::locLineNr() const
{
    return mLocLineNr;
}

int Result::locCol() const
{
    return mLocCol;
}

QString Result::locFile() const
{
    return mLocFile;
}

QString Result::node() const
{
    return mNode;
}

Result::Result(int locLineNr, int locCol, QString locFile, QString node) :
    mLocLineNr(locLineNr), mLocCol(locCol), mLocFile(locFile), mNode(node)
{
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

void SearchDialog::selectNextMatch(SearchDirection direction, QList<Result> matches)
{
    QTextCursor matchSelection;
    QRegularExpression searchRegex;
    QFlags<QTextDocument::FindFlag> flags = getFlags();

    flags.setFlag(QTextDocument::FindBackward, direction == SearchDirection::Backward);

    QString searchTerm = ui->combo_search->currentText();
    int searchLength = searchTerm.length();

    if (regex()) searchRegex.setPattern(searchTerm);

    ProjectFileNode *fc = mMain->projectRepo()->fileNode(mMain->recent()->editor());
    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(mMain->recent()->editor());

    if (regex())
        matchSelection = fc->document()->find(searchRegex, edit->textCursor(), flags);
    else
        matchSelection = fc->document()->find(searchTerm, edit->textCursor(), flags);

    if (matches.size() > 0) { // has any matches at all

        if (matchSelection.isNull()) { // empty selection == reached end of document
            if (direction == SearchDirection::Forward) {
                edit->setTextCursor(QTextCursor(edit->document())); // start from top
            } else {
                QTextCursor tc(edit->document());
                tc.movePosition(QTextCursor::End); // start from bottom
                edit->setTextCursor(tc);
            }
            selectNextMatch(direction, matches);

        } else { // found next match
            edit->setTextCursor(matchSelection);
        }
    } else {
        setSearchStatus(SearchStatus::NoResults);
        QTextCursor tc = edit->textCursor();
        tc.clearSelection();
        edit->setTextCursor(tc);
        return; // search had no matches so do nothing at all
    }

    // set match and counter
    int count = 0;
    foreach (Result match, matches) {
        if (match.locLineNr() == matchSelection.blockNumber()+1
                && match.locCol() == matchSelection.columnNumber() - searchLength) {
            updateMatchAmount(matches.size(), count+1);
            break;
        } else {
            count++;
        }
    }
}

void SearchDialog::on_btn_clear_clicked()
{
    clearSearch();
}

void SearchDialog::clearResults()
{
    // TODO: maybe this should remove matches in all files
    ProjectFileNode *fc = mMain->projectRepo()->fileNode(mMain->recent()->editor());
    if (!fc) return;
    fc->removeTextMarks(TextMark::match, true);
    setSearchStatus(SearchStatus::Clear);
}

void SearchDialog::clearSearch()
{
    ui->combo_search->clearEditText();
    ui->txt_replace->clear();

    clearResults();
}


void SearchDialog::on_combo_search_currentTextChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    mHasChanged = true;
    setSearchStatus(SearchStatus::Clear);
    clearResults();
    searchParameterChanged();

// removed due to performance issues in larger files:
//    clearResults();
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

void SearchDialog::searchParameterChanged() {
    setSearchStatus(SearchDialog::Clear);
    invalidateCache();
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

void SearchDialog::on_cb_caseSens_stateChanged(int state)
{
    QCompleter *completer = ui->combo_search->completer();
    if (Qt::Checked == state)
        completer->setCaseSensitivity(Qt::CaseSensitive);
    else
        completer->setCaseSensitivity(Qt::CaseInsensitive);

    searchParameterChanged();
}

}
}
