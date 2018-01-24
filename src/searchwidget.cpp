/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
    StudioSettings *mSettings = mMain->settings();

    ui->setupUi(this);
    ui->cb_regex->setChecked(mSettings->searchUseRegex());
    ui->cb_caseSens->setChecked(mSettings->searchCaseSens());
    ui->cb_wholeWords->setChecked(mSettings->searchWholeWords());
    ui->combo_scope->setCurrentIndex(mSettings->selectedScopeIndex());
    ui->lbl_nrResults->setText("");

    setFixedSize(size());
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
    return ui->cmb_search->currentData(Qt::DisplayRole).toString();
}

int SearchWidget::selectedScope()
{
    return ui->combo_scope->currentIndex();
}

void SearchWidget::setSelectedScope(int index)
{
    ui->combo_scope->setCurrentIndex(index);
}

void SearchWidget::on_btn_ReplaceAll_clicked()
{
//    switch (ui->combo_scope->currentIndex()) {
    simpleReplaceAll();
//    case 0: // this file
//        break;
//    case 1: // this group
//        EXCEPT() << "Not implemented yet";
////        replaceInGroup();
//        break;
//    case 2: // open files
//        EXCEPT() << "Not implemented yet";
////        replaceInOpenFiles();
//        break;
//    case 3: // all files/groups
//        EXCEPT() << "Not implemented yet";
//        break;
//    default:
//        break;
//    }
}

void SearchWidget::on_btn_FindAll_clicked()
{
    SearchResultList matches(searchTerm());

    switch (ui->combo_scope->currentIndex()) {
    case 0: // this file
        if (mMain->recent()->editor)
            matches.addResultList(findInFile(mMain->fileRepository()->fileContext(mMain->recent()->editor)));
        break;
    case 1: // this group
        matches.addResultList(findInGroup());
        break;
    case 2: // open files
        matches.addResultList(findInOpenFiles());
        break;
    case 3: // all files/groups
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
        fgc = (fc ? fc->parentEntry() : nullptr);

        if (!fgc)
            return QList<Result>();
    } else {
        if (fsc->type() == FileGroupContext::FileGroup)
            fgc = static_cast<FileGroupContext*>(fsc);
    }

    if (!fgc)
        return matches;

    for (int i = 0; i < fgc->childCount(); i++) {
        matches.append(findInFile(fgc->childEntry(i)));
    }
    return matches;
}

QList<Result> SearchWidget::findInFile(FileSystemContext *fsc)
{
    QRegExp rx(ui->txt_filePattern->text());
    rx.setPatternSyntax(QRegExp::Wildcard);

    // (scope not current file && wildcard not matching) || has gdx extension
    if (((ui->combo_scope->currentIndex() != 1) && (rx.indexIn(fsc->location()) == -1))
            || fsc->location().endsWith("gdx")) {
        return QList<Result>();
    }

    QString searchTerm = ui->cmb_search->currentData(Qt::DisplayRole).toString();
    SearchResultList matches(searchTerm);
    if (regex()) matches.useRegex(true);

    if (fsc->type() == FileSystemContext::FileGroup) { // or is it a group?
        matches.addResultList(findInGroup(fsc)); // TESTME studio does not support this case as of yet
    } else { // it's a file
        FileContext *fc(static_cast<FileContext*>(fsc));
        if (fc == nullptr) FATAL();

        QRegularExpression searchRegex = QRegularExpression(searchTerm);
        QTextCursor item;
        QTextCursor lastItem;

        if (fc->document() == nullptr) { // not opened in editor

            int lineCounter = 0;
            Qt::CaseSensitivity cs = (ui->cb_caseSens->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);

            if (regex()) {
                if (!caseSens()) searchRegex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }

            QFile file(fc->location());
            if (file.open(QIODevice::ReadOnly)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    lineCounter++;
                    QString line = in.readLine();
                    QRegularExpressionMatch match;

                    if (regex() && line.contains(searchRegex, &match)) {
                        matches.addResult(lineCounter, match.capturedEnd(), file.fileName(), line.trimmed());
                    } else if (line.contains(searchTerm, cs)){
                        matches.addResult(lineCounter, line.indexOf(searchTerm, cs), file.fileName(), line.trimmed());
                    }
                }
                file.close();
            }

        } else { // read from editor document(s)

            // if currently in foreground
            bool isOpenFile = (fc == mMain->fileRepository()->fileContext(mMain->recent()->editor));

            lastItem = QTextCursor(fc->document());
            do {
                if (regex())
                    item = fc->document()->find(searchRegex, lastItem, getFlags());
                else
                    item = fc->document()->find(searchTerm, lastItem, getFlags());

                if (item != lastItem) lastItem = item;
                else break;

                if (!item.isNull()) {
                    matches.addResult(item.blockNumber()+1, item.columnNumber(), fc->location(), item.block().text().trimmed());
                    if (isOpenFile) {
                        int length = item.selectionEnd() - item.selectionStart();
                        mAllTextMarks.append(fc->generateTextMark(TextMark::result, 0, item.blockNumber(),
                                                                  item.columnNumber() - length, length));
                    }
                }

            } while (!item.isNull());
            if (fc->highlighter()) fc->highlighter()->rehighlight();
        }
    }

    return matches.resultList();
}

void SearchWidget::updateMatchAmount(int hits, bool clear)
{
    if (clear) {
        ui->lbl_nrResults->setText("");
        ui->lbl_nrResults->setFrameShape(QFrame::NoFrame);
        return;
    }
    if (hits == 1)
        ui->lbl_nrResults->setText(QString::number(hits) + " match");
    else
        ui->lbl_nrResults->setText(QString::number(hits) + " matches");
    ui->lbl_nrResults->setFrameShape(QFrame::StyledPanel);
}

void SearchWidget::simpleReplaceAll()
{
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mMain->recent()->editor);
    if (!edit) return;

    QString searchTerm = ui->cmb_search->currentData(Qt::DisplayRole).toString();
    QRegularExpression searchRegex(ui->cmb_search->currentData(Qt::DisplayRole).toString());
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
        if (!item.isNull()) {
            hits.append(item);
        }
    } while (!item.isNull());

    QMessageBox msgBox;
    msgBox.setText("Replacing " + QString::number(hits.length()) + " occurrences of '" +
                   searchTerm + "' with '" + replaceTerm + "'. Are you sure?");
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

void SearchWidget::find(bool backwards)
{
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mMain->recent()->editor);
    if (!edit) return;

    bool useRegex = ui->cb_regex->isChecked();
    QString searchTerm = ui->cmb_search->currentData(Qt::DisplayRole).toString();
    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();
    if (backwards)
        searchFlags.setFlag(QTextDocument::FindFlag::FindBackward);

    QRegularExpression searchRegex;
    if (useRegex) searchRegex.setPattern(searchTerm);

    mLastSelection = (!mSelection.isNull() ? mSelection : edit->textCursor());
    if (useRegex) {
        mSelection = edit->document()->find(searchRegex, mLastSelection, searchFlags);
    } else {
        mSelection = edit->document()->find(searchTerm, mLastSelection, searchFlags);
    }

    // nothing found, try to start over
    if (mSelection.isNull()) {
        if (useRegex) {
            mSelection = edit->document()->find(searchRegex, 0, searchFlags);
        } else {
            mSelection = edit->document()->find(searchTerm, 0, searchFlags);
        }
    }

    // on hit
    if (!mSelection.isNull()) {
        edit->setTextCursor(mSelection);
    }
}

void SearchWidget::on_btn_Replace_clicked()
{
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mMain->recent()->editor);
    if (!edit) return;

    QString replaceTerm = ui->txt_replace->text();
    if (edit->textCursor().hasSelection())
        edit->textCursor().insertText(replaceTerm);

    find();
}

void SearchWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mMain->recent()->editor);
    if (!edit) return;

    ui->cmb_search->setFocus();
    if (edit->textCursor().hasSelection())
        ui->cmb_search->setCurrentText(edit->textCursor().selection().toPlainText());
    else
        ui->cmb_search->setCurrentText("");
}

void SearchWidget::keyPressEvent(QKeyEvent* event)
{
    if (isVisible() && ( event->key() == Qt::Key_Escape
                         || (event->modifiers() & Qt::ControlModifier && (event->key() == Qt::Key_F)) )) {
        hide();

        if (mMain->recent()->editor)
            mMain->recent()->editor->setFocus();
    } else if (event->modifiers() & Qt::ShiftModifier && (event->key() == Qt::Key_F3)) {
        find(true);
    } else if (event->key() == Qt::Key_F3) {
        find();
    } else if (event->key() == Qt::Key_Return) {
        on_btn_forward_clicked();
    }
}

void SearchWidget::closeEvent(QCloseEvent *event) {
    updateMatchAmount(0, true);

    QDialog::closeEvent(event);
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
    ui->txt_filePattern->setEnabled(index != 0);
}

void SearchWidget::on_btn_back_clicked()
{
    find(true);
}

void SearchWidget::on_btn_forward_clicked()
{
    FileContext *fc = mMain->fileRepository()->fileContext(mMain->recent()->editor);
    if (!fc) return;

    if (fc->textMarkCount(QSet<TextMark::Type>() << TextMark::result) == 0) { // if has no results search first
        on_btn_FindAll_clicked();
    }
    find(false);
}

void SearchWidget::on_btn_clear_clicked()
{
    FileContext *fc = mMain->fileRepository()->fileContext(mMain->recent()->editor);
    if (!fc) return;

    fc->removeTextMarks(TextMark::result);
    updateMatchAmount(0, true);
}

void SearchWidget::on_cmb_search_currentTextChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    FileContext *fc = mMain->fileRepository()->fileContext(mMain->recent()->editor);
    if (fc)
        fc->removeTextMarks(TextMark::result);
}

}
}
