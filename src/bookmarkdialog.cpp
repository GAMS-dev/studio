#include "bookmarkdialog.h"
#include "helpview.h"

namespace gams {
namespace studio {

BookmarkDialog::BookmarkDialog(QMultiMap<QString, QString>& bmMap, QWidget* parent):
    QDialog(parent), bookmarkMap(bmMap)
{
    ui.setupUi(this);

    model = new QStandardItemModel(bookmarkMap.size(), 2, this);
    ui.bookmarkTableView->verticalHeader()->hide();
    model->setHeaderData(0, Qt::Horizontal, QString("Name"));
    model->setHeaderData(1, Qt::Horizontal, QString("Location"));

    QMultiMap<QString, QString>::iterator it;
    int i = 0;
    for (it = bookmarkMap.begin(); it != bookmarkMap.end(); ++it) {
        QStandardItem* firstcol = new QStandardItem(it.value());
        model->setItem(i, 0, firstcol);
        QStandardItem* secondcol = new QStandardItem(it.key());
        model->setItem(i, 1, secondcol);
        model->item(i, 0)->setFlags( model->item(i, 0)->flags() ^ Qt::ItemIsEditable );
        model->item(i, 1)->setFlags( model->item(i, 1)->flags() ^ Qt::ItemIsEditable );
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
    connect(ui.bookmarkTableView, &QTableView::clicked, this, &BookmarkDialog::on_bookmarkEntryShowed);
    connect(ui.bookmarkTableView, &QTableView::customContextMenuRequested,
            this, &BookmarkDialog::on_contextMenuShowed);

    connect(this, &BookmarkDialog::openUrl, (HelpView*)parent, &HelpView::on_urlOpened );
    connect(this, &BookmarkDialog::removeBookmark, (HelpView*)parent, &HelpView::on_bookmarkRemoved );
    connect(this, &BookmarkDialog::updateBookmarkName, (HelpView*)parent, &HelpView::on_bookmarkNameUpdated );
    connect(this, &BookmarkDialog::updateBookmarkLocation, (HelpView*)parent, &HelpView::on_bookmarkLocationUpdated );
}

BookmarkDialog::~BookmarkDialog()
{

}

void BookmarkDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        if (focusWidget() == ui.nameLineEdit) {
            QModelIndexList selection = ui.bookmarkTableView->selectionModel()->selectedRows();
            if (selection.count() > 0) {
                model->setData( model->index(selection.at(0).row(), 0), ui.nameLineEdit->text());
                emit updateBookmarkName( model->data(model->index(selection.at(0).row(), 1)).toString(),
                                          model->data(model->index(selection.at(0).row(), 0)).toString() );
            }
        }
        if (focusWidget() == ui.locationLineEdit ) {
            QModelIndexList selection = ui.bookmarkTableView->selectionModel()->selectedRows();
            if (selection.count() > 0) {
                QString oldLocation = model->data(model->index(selection.at(0).row(), 1)).toString();
                model->setData(model->index(selection.at(0).row(), 1), ui.locationLineEdit->text());
                emit updateBookmarkLocation( oldLocation,
                                             model->data(model->index(selection.at(0).row(), 1)).toString(),
                                             model->data(model->index(selection.at(0).row(), 0)).toString() );
            }
        }
    } else {
        if (event->key() == Qt::Key_Escape) {
            QModelIndexList selection = ui.bookmarkTableView->selectionModel()->selectedRows();
            if (selection.count() > 0)
                on_bookmarkEntryShowed( selection.at(0) );

        }
    }
}

void BookmarkDialog::on_bookmarkEntryShowed(const QModelIndex &index)
{
   ui.nameLineEdit->setText( model->data(model->index(index.row(), 0), Qt::DisplayRole).toString() );
   ui.locationLineEdit->setText( model->data(model->index(index.row(), 1), Qt::DisplayRole).toString() );
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
           emit removeBookmark( ui.locationLineEdit->text(), ui.nameLineEdit->text() );

           QModelIndexList newSelection = ui.bookmarkTableView->selectionModel()->selectedRows();
           if (newSelection.count() > 0) {
               on_bookmarkEntryShowed( newSelection.at(0) );
           } else {
               ui.nameLineEdit->setText( "" );
               ui.locationLineEdit->setText( "" );
           }
        }
    }

}


} // namespace studio
} // namespace gams
