#ifndef GAMS_STUDIO_TABDIALOG_H
#define GAMS_STUDIO_TABDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

namespace Ui {
class TabDialog;
}

namespace gams {
namespace studio {

class TabListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    TabListModel(QTabWidget *tabs);
    virtual ~TabListModel() override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QTabWidget *tabs() { return mTabs; }

private:
    QTabWidget *mTabs = nullptr;

private:
    QString nameAppendix(const QModelIndex &index) const;
};

class TabDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TabDialog(QTabWidget *tabs, QWidget *parent = nullptr);
    ~TabDialog() override;
protected:
    void showEvent(QShowEvent *e) override;
    void resizeToContent();
    void keyPressEvent(QKeyEvent *e) override;
private slots:
    void setFilter(const QString &filter);
    void returnPressed();
    void selectTab(const QModelIndex &index);
private:
    Ui::TabDialog *ui;
    TabListModel *mTabModel;
    QSortFilterProxyModel *mFilterModel;
};


} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_TABDIALOG_H
