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
#include "searchwidget.h"
#include "studiosettings.h"
#include "syntax.h"
#include "ui_searchwidget.h"
#include "exception.h"
#include "searchresultlist.h"
#include <QDebug>

namespace gams {
namespace studio {

SearchWidget::SearchWidget(MainWindow *parent) :
    QDialog(parent), ui(new Ui::SearchWidget), mMain(parent)
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
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

bool SearchWidget::regex()
{
    return ui->cb_regex->isChecked();
}

bool SearchWidget::caseSens()
{
    return ui->cb_caseSens->isChecked();
}

bool SearchWidget::wholeWords()
{
    return ui->cb_wholeWords->isChecked();
}

QString SearchWidget::searchTerm()
{
    return ui->combo_search->currentText();
}

int SearchWidget::selectedScope()
{
    return ui->combo_scope->currentIndex();
}

void SearchWidget::setSelectedScope(int index)
{
    ui->combo_scope->setCurrentIndex(index);
}

void SearchWidget::on_btn_Replace_clicked()
{
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mMain->recent()->editor);
    if (!edit || edit->isReadOnly()) return;

    QString replaceTerm = ui->txt_replace->text();
    if (edit->textCursor().hasSelection())
        edit->textCursor().insertText(replaceTerm);

    findNext(SearchWidget::Forward);
}

void SearchWidget::on_btn_ReplaceAll_clicked()
{
    // TODO: allow users to replace in more than the current file?
    simpleReplaceAll();
}

void SearchWidget::on_btn_FindAll_clicked()
{
    SearchResultList matches(searchTerm());
    insertHistory();

    setSearchStatus();

    switch (ui->combo_scope->currentIndex()) {
    case SearchScope::ThisFile:
        if (mMain->recent()->editor)
            matches.addResultList(findInFile(mMain->fileRepository()->fileContext(mMain->recent()->editor)));
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

QList<Result> SearchWidget::findInAllFiles()
{
    QList<Result> matches;
    FileGroupContext *root = mMain->fileRepository()->treeModel()->rootContext();
    FileSystemContext *fsc;
    for (int i = 0; i < root->childCount(); i++) {
        fsc = root->childEntry(i);
        if (fsc->type() == FileSystemContext::ContextType::FileGroup)
            matches.append(findInGroup(fsc));
    }
    return matches;
}

QList<Result> SearchWidget::findInOpenFiles()
{
    QList<Result> matches;
    QWidgetList editList = mMain->fileRepository()->editors();
    FileContext *fc;
    // TODO: search FCs, because 1 fc can have n editors
    for (int i = 0; i < editList.size(); i++) {
        fc = mMain->fileRepository()->fileContext(editList.at(i));
        if (fc == nullptr) break;
        matches.append(findInFile(fc));
    }
    return matches;
}

QList<Result> SearchWidget::findInGroup(FileSystemContext *fsc)
{
    QList<Result> matches;

    FileGroupContext *fgc = nullptr;
    if (!fsc) {
        FileContext* fc = mMain->fileRepository()->fileContext(mMain->recent()->editor);
        fgc = (fc ? fc->parentEntry() : nullptr); // TODO: refactor

        if (!fgc) return QList<Result>();
    } else {
        if (fsc->type() == FileGroupContext::FileGroup)
            fgc = static_cast<FileGroupContext*>(fsc);
    }

    if (!fgc) return matches;

    for (int i = 0; i < fgc->childCount(); i++) {
        matches.append(findInFile(fgc->childEntry(i)));
    }
    return matches;
}

QList<Result> SearchWidget::findInFile(FileSystemContext *fsc)
{
    if (!fsc) return QList<Result>();

    QRegExp fileFilter(ui->combo_filePattern->currentText().trimmed());
    fileFilter.setPatternSyntax(QRegExp::Wildcard);

    // (scope not current file && wildcard not matching) || has gdx extension
    if (((ui->combo_scope->currentIndex() != SearchScope::ThisFile) && (fileFilter.indexIn(fsc->location()) == -1))
            || fsc->location().endsWith("gdx")) {
        return QList<Result>(); // dont search here, return empty
    }

    QString searchTerm = ui->combo_search->currentText();
    int searchLength = searchTerm.length();
    SearchResultList matches(searchTerm);
    if (regex()) matches.useRegex(true);

    if (fsc->type() == FileSystemContext::FileGroup) { // or is it a group?
        matches.addResultList(findInGroup(fsc)); // TESTME studio does not support this case as of yet
    } else { // it's a file
        FileContext *fc(static_cast<FileContext*>(fsc));
        if (fc == nullptr) FATAL() << "FileContext not found";

        QRegularExpression searchRegex;

        if (!regex()) searchRegex.setPattern(QRegularExpression::escape(searchTerm));
        else searchRegex.setPattern(searchTerm);

        if (wholeWords()) searchRegex.setPattern("\\b" + searchRegex.pattern() + "\\b");
        if (!caseSens()) searchRegex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

        QTextCursor item;
        QTextCursor lastItem;

        if (!fc->document()) { // not opened in editor

            int lineCounter = 0;

            QFile file(fc->location());
            if (file.open(QIODevice::ReadOnly)) {
                QTextStream in(&file);
                while (!in.atEnd()) { // read file
                    lineCounter++;
                    QString line = in.readLine();
                    QRegularExpressionMatch match;

                    int count = line.count(searchRegex);
                    if (count > 0) {
                        for (int i = 0; i < count; i++) {
                            matches.addResult(lineCounter, match.capturedEnd() - searchLength,
                                              file.fileName(), line.trimmed());
                        }
                    }
                }
                file.close();
            }
        } else { // read from editor document(s)

            // if currently in foreground
            bool isOpenFile = (fc == mMain->fileRepository()->fileContext(mMain->recent()->editor));

            lastItem = QTextCursor(fc->document());
            do {
                item = fc->document()->find(searchRegex, lastItem, getFlags());

                if (item != lastItem) lastItem = item;
                else break;

                if (!item.isNull()) {
                    matches.addResult(item.blockNumber()+1, item.columnNumber() - searchTerm.length(),
                                      fc->location(), item.block().text().trimmed());
                    if (isOpenFile) {
                        int length = item.selectionEnd() - item.selectionStart();
                        fc->generateTextMark(TextMark::match, 0, item.blockNumber(),
                                             item.columnNumber() - length, length);
                    }
                }
            } while (!item.isNull());
            if (fc->highlighter()) fc->highlighter()->rehighlight();
        }
    }
    return matches.resultList();
}

void SearchWidget::updateMatchAmount(int hits, int current, bool clear)
{
    if (clear) {
        ui->lbl_nrResults->setText("");
        ui->lbl_nrResults->setFrameShape(QFrame::NoFrame);
        return;
    }

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

void SearchWidget::simpleReplaceAll()
{
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mMain->recent()->editor);
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
                       + mMain->fileRepository()->fileContext(mMain->recent()->editor)->location()
                       + ". Are you sure?");
    } else {
        msgBox.setText("Replacing " + QString::number(hits.length()) + " occurrences of '" +
                       searchTerm + "' with '" + replaceTerm + "' in file "
                       + mMain->fileRepository()->fileContext(mMain->recent()->editor)->location()
                       + ". Are you sure?");
    }
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int answer = msgBox.exec();

    if (answer == QMessageBox::Ok) {
        edit->textCursor().beginEditBlock();
        foreach (QTextCursor tc, hits) {
            tc.insertText(replaceTerm);
        }
        edit->textCursor().endEditBlock();
    }
}

void SearchWidget::setSearchStatus()
{
    ui->lbl_nrResults->setText("Searching...");
    ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);
    QApplication::processEvents(QEventLoop::AllEvents, 20);
}

void SearchWidget::findNext(SearchDirection direction)
{
    if (!mMain->recent()->editor) return;

    FileContext *fc = mMain->fileRepository()->fileContext(mMain->recent()->editor);
    if (!fc) return;
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mMain->recent()->editor);
    if (!edit) return;

    if (hasChanged) {
        setSearchStatus();
        cachedResults = findInFile(fc);
        hasChanged = false;
    }

    selectNextMatch(direction, cachedResults);
}

void SearchWidget::focusSearchField()
{
    ui->combo_search->setFocus();
}

void SearchWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    QWidget *widget = mMain->recent()->editor;
    QPlainTextEdit *edit = FileContext::toPlainEdit(widget);
    FileSystemContext *fsc = mMain->fileRepository()->fileContext(widget);
    if (!fsc || !edit) return;

    focusSearchField();
    if (edit->textCursor().hasSelection())
        ui->combo_search->setCurrentText(edit->textCursor().selection().toPlainText());
    else
        ui->combo_search->setCurrentText("");

    updateReplaceActionAvailability();
}

void SearchWidget::updateReplaceActionAvailability()
{
    QPlainTextEdit *edit = FileContext::toPlainEdit(mMain->recent()->editor);
    bool isSourceCode = FileContext::editorType(mMain->recent()->editor) == FileSystemContext::etSourceCode;

    bool activateSearch = isSourceCode || FileContext::editorType(mMain->recent()->editor) == FileSystemContext::etLxiLst;
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

    ui->combo_filePattern->setEnabled(activateSearch);
    ui->combo_scope->setEnabled(activateSearch);
}

void SearchWidget::on_searchNext()
{
    findNext(SearchWidget::Forward);
}

void SearchWidget::on_searchPrev()
{
    findNext(SearchWidget::Backward);
}

void SearchWidget::keyPressEvent(QKeyEvent* e)
{
    if ( isVisible() && (e->key() == Qt::Key_Escape) ) {
        hide();
        if (mMain->fileRepository()->fileContext(mMain->recent()->editor))
            mMain->recent()->editor->setFocus();

    } else if (e->modifiers() & Qt::ShiftModifier && (e->key() == Qt::Key_F3)) {
        on_searchPrev();
    } else if (e->key() == Qt::Key_F3) {
        on_searchNext();
    }
    QDialog::keyPressEvent(e);
}

void SearchWidget::closeEvent(QCloseEvent *e) {
    updateMatchAmount(0, true);
    QDialog::closeEvent(e);
}

QFlags<QTextDocument::FindFlag> SearchWidget::getFlags()
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

QString Result::context() const
{
    return mContext;
}

Result::Result(int locLineNr, int locCol, QString locFile, QString context) :
    mLocLineNr(locLineNr), mLocCol(locCol), mLocFile(locFile), mContext(context)
{
}

void SearchWidget::on_combo_scope_currentIndexChanged(int index)
{
    ui->combo_filePattern->setEnabled(index != SearchScope::ThisFile);
}

void SearchWidget::on_btn_back_clicked()
{
    insertHistory();
    findNext(SearchWidget::Backward);
}

void SearchWidget::on_btn_forward_clicked()
{
    insertHistory();
    findNext(SearchWidget::Forward);
}

void SearchWidget::selectNextMatch(SearchDirection direction, QList<Result> matches)
{
    QTextCursor matchSelection;
    QRegularExpression searchRegex;
    QFlags<QTextDocument::FindFlag> flags = getFlags();

    flags.setFlag(QTextDocument::FindBackward, direction == SearchDirection::Backward);

    QString searchTerm = ui->combo_search->currentText();
    int searchLength = searchTerm.length();

    if (regex()) searchRegex.setPattern(searchTerm);

    FileContext *fc = mMain->fileRepository()->fileContext(mMain->recent()->editor);
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mMain->recent()->editor);

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
        return; // search had no matches anyway, so do nothing at all
    }

    // set match and counter
    int count = 0;
    foreach (Result match, matches) {
        if (matches.at(count).locLineNr() == matchSelection.blockNumber()+1
                && matches.at(count).locCol() == matchSelection.columnNumber() - searchLength) {
            updateMatchAmount(matches.size(), count+1);
            break;
        } else {
            count++;
        }
    }
}

void SearchWidget::on_btn_clear_clicked()
{
    clearResults();
}

void SearchWidget::clearResults()
{
    ui->combo_search->clearEditText();
    ui->txt_replace->clear();

    FileContext *fc = mMain->fileRepository()->fileContext(mMain->recent()->editor);
    if (!fc) return;

    fc->removeTextMarks(TextMark::match, false);
    updateMatchAmount(0, 0, true);
}


void SearchWidget::on_combo_search_currentTextChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    hasChanged = true;

// removed due to performance issues in larger files:
//    FileContext *fc = mMain->fileRepository()->fileContext(mMain->recent()->editor);
//    if (fc)
//        fc->removeTextMarks(TextMark::match);
}

void SearchWidget::insertHistory()
{
    QString searchText(ui->combo_search->currentText());
    if (ui->combo_search->findText(searchText) == -1) {
        ui->combo_search->insertItem(0, searchText);
    } else {
        bool state = hasChanged;
        ui->combo_search->removeItem(ui->combo_search->findText(searchText));
        ui->combo_search->insertItem(0, searchText);
        ui->combo_search->setCurrentIndex(0);
        hasChanged = state;
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

}
}
