#include "bookmarkdialog.h"
#include "helpview.h"

namespace gams {
namespace studio {

BookmarkDialog::BookmarkDialog(QMap<QString, QString>& bookmarkMap, QWidget* parent):
    QDialog(parent)
{
    ui.setupUi(this);

    model = new QStandardItemModel(bookmarkMap.size(), 2, this);
    ui.bookmarkTableView->horizontalHeader()->hide();
    ui.bookmarkTableView->verticalHeader()->hide();

    QMap<QString, QString>::iterator it;
    int i = 0;
    for (it = bookmarkMap.begin(); it != bookmarkMap.end(); ++it) {
        QStandardItem* firstcol = new QStandardItem(it.value());
        model->setItem(i, 0, firstcol);
        QStandardItem* secondcol = new QStandardItem(it.key());
        model->setItem(i, 1, secondcol);
        ++i;
    }

    ui.bookmarkTableView->setModel( model );
    ui.bookmarkTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.bookmarkTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.bookmarkTableView->setAutoScroll(true);
    ui.bookmarkTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.bookmarkTableView->horizontalHeader()->setStretchLastSection(true);
    ui.bookmarkTableView->resizeColumnsToContents();
    ui.bookmarkTableView->setAlternatingRowColors(true);

    ui.bookmarkTableView->setColumnHidden(1, true);

    connect(ui.bookmarkTableView, &QTableView::clicked,
            this, &BookmarkDialog::on_bookmarkEntryShowed);
    connect(ui.bookmarkTableView, &QTableView::customContextMenuRequested,
            this, &BookmarkDialog::on_contextMenuShowed);
    connect(this, &BookmarkDialog::openUrl, (HelpView*)parent, &HelpView::on_urlOpened );
    connect(this, &BookmarkDialog::removeBookmark, (HelpView*)parent, &HelpView::on_bookmarkRemoved );
}

BookmarkDialog::~BookmarkDialog()
{

}

void BookmarkDialog::on_bookmarkEntryShowed(const QModelIndex &index)
{
   ui.bookmarkTableView->setColumnHidden(1, true);
   ui.nameLineEdit->setText( model->data(index, Qt::DisplayRole).toString() );
   ui.locationLineEdit->setText( model->data(index.sibling(index.row(), 1), Qt::DisplayRole).toString() );
   ui.nameLineEdit->setCursorPosition(0);
   ui.locationLineEdit->setCursorPosition(0);
}

void BookmarkDialog::on_contextMenuShowed(const QPoint &pos)
{
    QModelIndexList selection = ui.bookmarkTableView->selectionModel()->selectedRows();

    QMenu menu(this);
    QAction* openUrlAction = menu.addAction("Load bookmark");
    menu.addSeparator();
    QAction* deleteAction = menu.addAction("Delete selected bookmark");

    if (selection.count() <= 0) {
        openUrlAction->setVisible(false);
        deleteAction->setVisible(false);
    }

    QAction* action = menu.exec(ui.bookmarkTableView->viewport()->mapToGlobal(pos));
    if (action == openUrlAction) {
        emit openUrl( QUrl(ui.locationLineEdit->text()) );
   } else if (action == deleteAction) {
        if (selection.count() > 0) {
           QModelIndex index = selection.at(0);
           ui.bookmarkTableView->model()->removeRow(index.row(), QModelIndex());
           emit removeBookmark( QUrl(ui.locationLineEdit->text()) );

           QModelIndexList newSelection = ui.bookmarkTableView->selectionModel()->selectedRows();
           if (newSelection.count() > 0) {
               on_bookmarkEntryShowed( newSelection.at(0) );
           } else {
               ui.bookmarkTableView->setColumnHidden(1, true);
               ui.nameLineEdit->setText( "" );
               ui.locationLineEdit->setText( "" );
           }
        }
    }

}

} // namespace studio
} // namespace gams
