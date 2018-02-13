#ifndef BOOKMARKDIALOG_H
#define BOOKMARKDIALOG_H

#include <QtWidgets>

#include "ui_bookmarkdialog.h"

namespace gams {
namespace studio {

class BookmarkDialog : public QDialog
{
    Q_OBJECT
public:
    BookmarkDialog(QMap<QString, QString>& bookmarkMap, QWidget *parent = 0);
    ~BookmarkDialog();

signals:
    void openUrl(const QUrl& location);
    void updateBookmark(const QUrl& location, const QString& name);
    void removeBookmark(const QUrl& location);

private slots:
//    void on_bookmarkEntryUpdated(const QUrl& location, const QString& name);
    void on_bookmarkEntryShowed(const QModelIndex &index);
    void on_contextMenuShowed(const QPoint &pos);

private:
    Ui::bookmarkDialog ui;
    QStandardItemModel* model;
};

} // namespace studio
} // namespace gams

#endif // BOOKMARKDIALOG_H
