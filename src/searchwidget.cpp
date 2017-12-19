#include "searchwidget.h"
#include "ui_searchwidget.h"
#include <QDebug>

namespace gams {
namespace studio {

SearchWidget::SearchWidget(RecentData &rec, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SearchWidget), mRecent(rec)
{
    ui->setupUi(this);
    ui->cb_caseSens->setProperty("flag", QTextDocument::FindCaseSensitively);
    ui->cb_wholeWords->setProperty("flag", QTextDocument::FindWholeWords);
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

void SearchWidget::on_btn_Find_clicked()
{
    QString searchTerm = ui->txt_search->text();

    mLastSelection = (!mSelection.isNull() ? mSelection : mRecent.editor->textCursor());

    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();

    mSelection = mRecent.editor->document()->find(searchTerm, mLastSelection, searchFlags);

    if (mSelection.isNull())
        mSelection = mRecent.editor->document()->find(searchTerm, 0, searchFlags); // try to start over

    if (!mSelection.isNull())
        mRecent.editor->setTextCursor(mSelection);
}

void SearchWidget::on_btn_FindAll_clicked()
{
    QTextCursor item;
    QTextCursor lastItem;
    QString searchTerm = ui->txt_search->text();
    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();

    QList<QTextEdit::ExtraSelection> allFinds;
    do {
        item = mRecent.editor->document()->find(searchTerm, lastItem, searchFlags);
        lastItem = item;
        if (!item.isNull()) {
            QTextEdit::ExtraSelection es;
            es.format.setBackground(QColor(Qt::yellow)); // TODO: fix coloring
            es.cursor = item;
            allFinds.append(es);
        }
    } while (!item.isNull());
    mRecent.editor->textCursor().clearSelection();
    mRecent.editor->setExtraSelections(allFinds);
}

void SearchWidget::on_btn_Replace_clicked()
{

}

void SearchWidget::on_btn_ReplaceAll_clicked()
{

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
