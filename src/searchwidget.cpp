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
    mRecent.editor->textCursor().clearSelection();
    QString searchTerm = ui->txt_search->text();

    mLastSelectionPos = (mSelection.position() > 0 ? mSelection.position() : 0);

    QFlags<QTextDocument::FindFlag> searchFlags;
    searchFlags.setFlag(QTextDocument::FindCaseSensitively, ui->cb_caseSens->isChecked());
    searchFlags.setFlag(QTextDocument::FindWholeWords, ui->cb_wholeWords->isChecked());

    mSelection = mRecent.editor->document()->find(searchTerm, mLastSelectionPos, searchFlags);
    mRecent.editor->setTextCursor(mSelection);
}

void SearchWidget::on_btn_FindAll_clicked()
{

}

void SearchWidget::on_btn_Replace_clicked()
{

}

void SearchWidget::on_btn_ReplaceAll_clicked()
{

}

void SearchWidget::showEvent(QShowEvent *event)
{
    ui->txt_search->setFocus();
    if (mRecent.editor->textCursor().hasSelection())
        ui->txt_search->setText(mRecent.editor->textCursor().selection().toPlainText());
}

void SearchWidget::on_txt_search_returnPressed()
{
    on_btn_Find_clicked();
}

}
}
