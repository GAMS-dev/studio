#include "searchwidget.h"
#include "syntax.h"
#include "ui_searchwidget.h"
#include <QDebug>

namespace gams {
namespace studio {

SearchWidget::SearchWidget(RecentData &rec, FileRepository &repo, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SearchWidget), mRecent(rec), mRepo(repo)
{
    ui->setupUi(this);
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

void SearchWidget::on_btn_Find_clicked()
{

    QString searchTerm = ui->txt_search->text();
    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();

    mLastSelection = (!mSelection.isNull() ? mSelection : mRecent.editor->textCursor());
    mSelection = mRecent.editor->document()->find(searchTerm, mLastSelection, searchFlags);

    if (mSelection.isNull())
        mSelection = mRecent.editor->document()->find(searchTerm, 0, searchFlags); // try to start over

    if (!mSelection.isNull()) {
        mRecent.editor->setTextCursor(mSelection);
        ui->btn_Replace->setEnabled(true);
    }
}

void SearchWidget::on_btn_FindAll_clicked()
{
    return; // TODO: work in progress
    QString searchTerm = ui->txt_search->text();
    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();

    QTextCursor item;
    QTextCursor lastItem;
    FileContext *fc = mRepo.fileContext(mRecent.editor);
    do {
        item = mRecent.editor->document()->find(searchTerm, lastItem, searchFlags);
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
    mRecent.editor->textCursor().clearSelection();
    qDebug() << "marks" << mAllTextMarks;
}

void SearchWidget::on_btn_Replace_clicked()
{
    QString replaceTerm = ui->txt_replace->text();
    if (mRecent.editor->textCursor().hasSelection())
        mRecent.editor->textCursor().insertText(replaceTerm);

    on_btn_Find_clicked();
}

void SearchWidget::on_btn_ReplaceAll_clicked()
{
    QString searchTerm = ui->txt_search->text();
    QString replaceTerm = ui->txt_replace->text();
    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();

    QList<QTextCursor> hits;
    QTextCursor item;
    QTextCursor lastItem;
    do {
        item = mRecent.editor->document()->find(searchTerm, lastItem, searchFlags);
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

        foreach (QTextCursor tc, hits) {
            tc.insertText(replaceTerm);
        }
    }
}

void SearchWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    ui->txt_search->setFocus();
    if (mRecent.editor->textCursor().hasSelection())
        ui->txt_search->setText(mRecent.editor->textCursor().selection().toPlainText());
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
