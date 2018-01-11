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
#include <QDebug>

namespace gams {
namespace studio {

SearchWidget::SearchWidget(StudioSettings *settings, RecentData &rec, FileRepository &repo, MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::SearchWidget), mSettings(settings), mRecent(rec), mRepo(repo), mMain(parent)
{
    ui->setupUi(this);
    ui->cb_regex->setChecked(mSettings->searchUseRegex());
    ui->cb_caseSens->setChecked(mSettings->searchCaseSens());
    ui->cb_wholeWords->setChecked(mSettings->searchWholeWords());
    ui->combo_scope->setCurrentIndex(mSettings->selectedScopeIndex());
    ui->lbl_nrResults->setText("");
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
    return ui->txt_search->text();
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
    switch (ui->combo_scope->currentIndex()) {
    case 0: // this file
        simpleReplaceAll();
        break;
    case 1: // this group
        EXCEPT() << "Not implemented yet";
//        replaceInGroup();
        break;
    case 2: // open files
        EXCEPT() << "Not implemented yet";
//        replaceInOpenFiles();
        break;
    case 3: // all files/groups
        EXCEPT() << "Not implemented yet";
        break;
    default:
        break;
    }
}

void SearchWidget::on_btn_FindAll_clicked()
{
    QList<Result> matches;

    switch (ui->combo_scope->currentIndex()) {
    case 0: // this file
        if (mRecent.editor)
            matches = findInFile(mRepo.fileContext(mRecent.editor));
        break;
    case 1: // this group
        matches = findInGroup();
        break;
    case 2: // open files
        matches = findInOpenFiles();
        break;
    case 3: // all files/groups
        matches = findInAllFiles();
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

    FileGroupContext *fgc;
    if (!fsc) {
        FileContext* fc = mMain->fileRepository()->fileContext(mRecent.editor);
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
    QList<Result> matches;

    if (fsc->type() == FileSystemContext::FileGroup) { // or is it a group?
        matches.append(findInGroup(fsc)); // TESTME studio does not support this case as of yet
    } else { // it's a file
        FileContext *fc(static_cast<FileContext*>(fsc));
        if (fc == nullptr) FATAL();



        QString searchTerm = ui->txt_search->text();
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

                    if (regex() && line.contains(searchRegex)) {
                        matches << Result(lineCounter, file.fileName(), line.trimmed());
                    } else if (line.contains(searchTerm, cs)){
                        matches << Result(lineCounter, file.fileName(), line.trimmed());
                    }
                }
                file.close();
            }

        } else { // read from editor document(s)

            // currently in foreground
            bool highlightMatch = (fc == mRepo.fileContext(mRecent.editor));

            lastItem = QTextCursor(fc->document());
            do {
                if (regex())
                    item = fc->document()->find(searchRegex, lastItem, getFlags());
                else
                    item = fc->document()->find(searchTerm, lastItem, getFlags());

                if (item != lastItem) lastItem = item;
                else break;

                if (!item.isNull()) {
                    matches << Result(item.blockNumber()+1, fc->location(), item.block().text().trimmed());
                    if (highlightMatch) {
                        int length = item.selectionEnd() - item.selectionStart();
                        mAllTextMarks.append(fc->generateTextMark(TextMark::result, 0, item.blockNumber(),
                                                                  item.columnNumber() - length, length));
                    }
                }

            } while (!item.isNull());
            if (fc->highlighter()) fc->highlighter()->rehighlight();
        }
    }

    return matches;
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
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mRecent.editor);
    if (!edit) return;

    QString searchTerm = ui->txt_search->text();
    QString replaceTerm = ui->txt_replace->text();
    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();

    QList<QTextCursor> hits;
    QTextCursor item;
    QTextCursor lastItem;

    do {
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
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mRecent.editor);
    if (!edit) return;

    bool useRegex = ui->cb_regex->isChecked();
    QString searchTerm = ui->txt_search->text();
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
        ui->btn_Replace->setEnabled(true);
    }
}

void SearchWidget::on_btn_Replace_clicked()
{
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mRecent.editor);
    if (!edit) return;

    QString replaceTerm = ui->txt_replace->text();
    if (edit->textCursor().hasSelection())
        edit->textCursor().insertText(replaceTerm);

    find();
}

void SearchWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mRecent.editor);
    if (!edit) return;

    ui->txt_search->setFocus();
    if (edit->textCursor().hasSelection())
        ui->txt_search->setText(edit->textCursor().selection().toPlainText());
    else
        ui->txt_search->setText("");
}

void SearchWidget::keyPressEvent(QKeyEvent* event)
{
    if (isVisible() && ( event->key() == Qt::Key_Escape
                         || (event->modifiers() & Qt::ControlModifier && (event->key() == Qt::Key_F)) )) {
        hide();
        mRecent.editor->setFocus();
    }
}

void SearchWidget::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
    FileContext *fc = mRepo.fileContext(mRecent.editor);
    if (fc)
        fc->removeTextMarks(TextMark::result);

    updateMatchAmount(0, true);
}


void SearchWidget::on_txt_search_returnPressed()
{
    on_btn_forward_clicked();
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

QString Result::locFile() const
{
    return mLocFile;
}

QString Result::context() const
{
    return mContext;
}

Result::Result(int locLineNr, QString locFile, QString context) :
    mLocLineNr(locLineNr), mLocFile(locFile), mContext(context)
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
    FileContext *fc = mRepo.fileContext(mRecent.editor);
    if (!fc) return;

    if (fc->textMarkCount(QSet<TextMark::Type>() << TextMark::result) == 0) { // if has no results search first
        on_btn_FindAll_clicked();
    }
    find(false);
}

void SearchWidget::on_btn_clear_clicked()
{
    FileContext *fc = mRepo.fileContext(mRecent.editor);
    if (!fc) return;

    fc->removeTextMarks(TextMark::result);
    updateMatchAmount(0, true);
}

}
}

// TODO: DEPRECATED, DELETEME
//QList<Result> SearchWidget::simpleFindAndHighlight(QPlainTextEdit* edit)
//{
//    QList<Result> matches;
//    if (edit == nullptr)
//        edit = FileSystemContext::toPlainEdit(mRecent.editor);
//    if (!edit) return matches;

//    QString searchTerm = ui->txt_search->text();
//    QRegularExpression searchRegex(searchTerm);
//    QTextCursor item;
//    QTextCursor lastItem;
//    FileContext *fc = mRepo.fileContext(mRecent.editor);
//    int hits = 0;

//    fc->removeTextMarks(TextMark::result);
//    QTextCursor tmpCurs = edit->textCursor();
//    tmpCurs.clearSelection();
//    edit->setTextCursor(tmpCurs);

//    do {
//        if (regex()) item = edit->document()->find(searchRegex, lastItem, getFlags());
//        else  item = edit->document()->find(searchTerm, lastItem, getFlags());

//        lastItem = item;
//        if (!item.isNull()) {
//            int length = item.selectionEnd() - item.selectionStart();
//            mAllTextMarks.append(fc->generateTextMark(TextMark::result, 0, item.blockNumber(),
//                                                      item.columnNumber() - length, length));
//            matches << Result(item.blockNumber()+1, fc->location(), item.block().text().trimmed());
//            hits++;
//        }
//    } while (!item.isNull());

//    if (fc->highlighter()) fc->highlighter()->rehighlight();
//    updateMatchAmount(hits);

//    return matches;
//}
