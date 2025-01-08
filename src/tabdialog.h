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
#ifndef GAMS_STUDIO_TABDIALOG_H
#define GAMS_STUDIO_TABDIALOG_H

#include <QDialog>
#include <QAbstractListModel>

class QTabWidget;
class QSortFilterProxyModel;

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
    QString nameAppendix(const QModelIndex &index) const;
    QString modName(const QModelIndex &index) const;

private:
    QTabWidget *mTabs = nullptr;
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
    bool eventFilter(QObject *watched, QEvent *event) override;

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
