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
    BookmarkDialog(QMultiMap<QString, QString>& bmMap, QWidget *parent = 0);
    ~BookmarkDialog();

protected:
    void keyPressEvent(QKeyEvent *event);

signals:
    void openUrl(const QUrl& location);
    void updateBookmarkName(const QString& location, const QString& name);
    void updateBookmarkLocation(const QString& oldLocation, const QString& newLocation, const QString& name);
    void removeBookmark(const QString& location, const QString& name);

private slots:
    void on_bookmarkEntryShowed(const QModelIndex &index);
    void on_contextMenuShowed(const QPoint &pos);

private:
    Ui::bookmarkDialog ui;
    QStandardItemModel* model;
    QMultiMap<QString, QString>& bookmarkMap;
};

} // namespace studio
} // namespace gams

#endif // BOOKMARKDIALOG_H
