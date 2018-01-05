#include "searchwidget.h"
#include "syntax.h"
#include "ui_searchwidget.h"
#include <QDebug>

namespace gams {
namespace studio {

SearchWidget::SearchWidget(RecentData &rec, FileRepository &repo, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchWidget), mRecent(rec), mRepo(repo)
{
    ui->setupUi(this);
}

SearchWidget::~SearchWidget()
{
    delete ui;
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

void SearchWidget::on_btn_Find_clicked()
{
    find();
}

void SearchWidget::on_btn_FindAll_clicked()
{
    return; // TODO: work in progress
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mRecent.editor);
    if (!edit) return;
    QString searchTerm = ui->txt_search->text();
    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();

    QTextCursor item;
    QTextCursor lastItem;
    FileContext *fc = mRepo.fileContext(mRecent.editor);
    do {
        item = edit->document()->find(searchTerm, lastItem, searchFlags);
        lastItem = item;
        if (!item.isNull()) {
            int length = item.selectionEnd() - item.selectionStart();
            qDebug() << "generating hit for lineNr" << item.blockNumber()-1
                     << "col" << item.columnNumber() - length
                     << "size" << length;

            mAllTextMarks.append(fc->generateTextMark(TextMark::result, 0, item.blockNumber()-1,
                                                      item.columnNumber() - length, length));
        }
    } while (!item.isNull());
    edit->textCursor().clearSelection();
    qDebug() << "marks" << mAllTextMarks;
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

void SearchWidget::on_btn_ReplaceAll_clicked()
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
                         || (Qt::ControlModifier && (event->key() == Qt::Key_F)) )) {
        hide();
        mRecent.editor->setFocus();
    }
    if (isVisible() && event->modifiers() & Qt::ShiftModifier && event->key() == Qt::Key_F3) {
        find(true);
    } else if (isVisible() && event->key() == Qt::Key_F3) {
        find();
    }
}


void SearchWidget::on_txt_search_returnPressed()
{
    on_btn_Find_clicked();
}

QFlags<QTextDocument::FindFlag> SearchWidget::getFlags()
{
    QFlags<QTextDocument::FindFlag> searchFlags;
    searchFlags.setFlag(QTextDocument::FindCaseSensitively, ui->cb_caseSens->isChecked());
    searchFlags.setFlag(QTextDocument::FindWholeWords, ui->cb_wholeWords->isChecked());

    return searchFlags;
}

}
}
