#ifndef MIROMODELASSEMBLYDIALOG_H
#define MIROMODELASSEMBLYDIALOG_H

#include <QDialog>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QSet>

namespace Ui {
class MiroModelAssemblyDialog;
}

namespace gams {
namespace studio {

class FilteredFileSystemModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FilteredFileSystemModel(QObject *parent = nullptr);

protected:
  bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;
};

class FileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    FileSystemModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QStringList selectedFiles();

//private:
//    QSet<QModelIndex> childs(const QModelIndex &parent);

private:
    QSet<QModelIndex> mCheckedIndexes;
};

class MiroModelAssemblyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MiroModelAssemblyDialog(const QString &workingDirectory, QWidget *parent = nullptr);
    ~MiroModelAssemblyDialog();

    QStringList selectedFiles();

private slots:
    void on_createButton_clicked();

private:
    void showMessageBox();

private:
    Ui::MiroModelAssemblyDialog *ui;
    FileSystemModel *mFileSystemModel;
};

}
}

#endif // MIROMODELASSEMBLYDIALOG_H
