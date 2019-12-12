#include "miromodelassemblydialog.h"
#include "ui_miromodelassemblydialog.h"

#include <QMessageBox>

namespace gams {
namespace studio {

FilteredFileSystemModel::FilteredFileSystemModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

bool FilteredFileSystemModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)

    if (source_column == 0)
        return true;
    return false;
}

FileSystemModel::FileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
}

QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::CheckStateRole) {
        if (mCheckedIndexes.contains(index)) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    }
    return QFileSystemModel::data(index, role);
}

bool FileSystemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 0) {
        if (value.toBool()) {
            mCheckedIndexes.insert(index);
//            mCheckedIndexes.unite(childs(index));
            emit dataChanged(index,
                             match(index, Qt::DisplayRole, "*", -1, Qt::MatchWildcard | Qt::MatchRecursive).last());
            return true;
        } else {
            auto pos = mCheckedIndexes.find(index);
            mCheckedIndexes.erase(pos);
//            mCheckedIndexes.subtract(childs(index));
            emit dataChanged(index,
                             match(index, Qt::DisplayRole, "*", -1, Qt::MatchWildcard | Qt::MatchRecursive).last());
            return true;
        }
    }
    return  QFileSystemModel::setData(index, value, role);
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QStringList FileSystemModel::selectedFiles()
{
    QStringList selection;
    for (auto index: mCheckedIndexes)
        selection << index.data().toString();
    return selection;
}

// TODO (AF) read files.txt and check selection
//QSet<QModelIndex> FileSystemModel::childs(const QModelIndex &parent)
//{
//    qDebug() << "#### ROWC >> " << rowCount(parent);

//    QSet<QModelIndex> subTree;
//    for (int i=0; i<rowCount(parent); ++i) {
//        auto child = index(i, 0, parent);
//        if (rowCount(child))
//            subTree.unite(childs(child));
//        subTree.insert(child);
//        qDebug() << "#### >> " << child.data().toString();
//    }

//    return subTree;
//}

MiroModelAssemblyDialog::MiroModelAssemblyDialog(const QString &workingDirectory, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MiroModelAssemblyDialog),
    mFileSystemModel(new FileSystemModel(this))
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mFileSystemModel->setRootPath(workingDirectory);
    auto filterModel = new FilteredFileSystemModel(this);
    filterModel->setSourceModel(mFileSystemModel);

    auto oldModel = ui->directoryView->selectionModel();
    ui->directoryView->setModel(filterModel);
    delete oldModel;

    auto rootIndex = mFileSystemModel->index(workingDirectory);
    ui->directoryView->setRootIndex(filterModel->mapFromSource(rootIndex));

    ui->directoryView->expandAll();
}

MiroModelAssemblyDialog::~MiroModelAssemblyDialog()
{
    delete ui;
}

QStringList MiroModelAssemblyDialog::selectedFiles()
{
    return mFileSystemModel->selectedFiles();
}

void MiroModelAssemblyDialog::on_createButton_clicked()
{
    if (selectedFiles().isEmpty()) {
        showMessageBox();
        return;
    }
    accept();
}

void MiroModelAssemblyDialog::showMessageBox()
{
    QMessageBox::critical(this, "No deployment files!", "Please select the files for your MIRO deployment.");
}

}
}
