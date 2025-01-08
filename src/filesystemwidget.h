/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_FS_FILESYSTEMWIDGET_H
#define GAMS_STUDIO_FS_FILESYSTEMWIDGET_H

#include <QGroupBox>
#include <QStyledItemDelegate>

namespace gams {
namespace studio {
namespace fs {

class FileSystemModel;
class FilteredFileSystemModel;

class FileSystemItemDelegate: public QStyledItemDelegate
{
public:
    FileSystemItemDelegate(QObject *parent = nullptr) { Q_UNUSED(parent) }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

namespace Ui {
class FileSystemWidget;
}

class FileSystemWidget : public QGroupBox
{
    Q_OBJECT
public:
    FileSystemWidget(QWidget *parent = nullptr);
    ~FileSystemWidget() override;
    void setInfo(const QString &message, bool valid);
    void setModelName(const QString &modelName);
    QStringList selectedFiles();
    void setSelectedFiles(const QStringList &files);
    QString workingDirectory() const;
    void setWorkingDirectory(const QString &workingDirectory);
    void clearSelection();
    void setupViewModel();
    bool showProtection() const { return mShowProtection; }
    void setShowProtection(bool showProtection);
    void setCreateVisible(bool visible);
    int selectionCount();
    void clearMissingFiles();
    QStringList missingFiles() { return mMissingFiles; }
    void selectFilter();

signals:
    void createClicked();
    void modified();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_createButton_clicked();
    void on_selectAllButton_clicked();
    void on_clearButton_clicked();
    void on_cbUncommon_clicked(bool checked);

    void selectionCountChanged(int count);
    void updateButtons();

private:
    Ui::FileSystemWidget *ui;
    bool mValidAssemblyFile;
    QString mWorkingDirectory;
    bool mShowProtection = false;
    QStringList mUncommonFiles;
    QStringList mMissingFiles;

    FileSystemModel *mFileSystemModel;
    FilteredFileSystemModel *mFilterModel;
    FileSystemItemDelegate *mDelegate;
};

}
}
}

#endif // GAMS_STUDIO_FS_FILESYSTEMWIDGET_H
