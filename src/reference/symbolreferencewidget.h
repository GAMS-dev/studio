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
#ifndef SYMBOLREFERENCEWIDGET_H
#define SYMBOLREFERENCEWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QItemSelection>

#include "reference.h"
#include "referencetreemodel.h"
#include "referenceviewer.h"
#include "symboldatatype.h"
#include "symboltablemodel.h"

namespace Ui {
class SymbolReferenceWidget;
}

namespace gams {
namespace studio {
namespace reference {

class ReferenceViewer;

class SymbolReferenceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SymbolReferenceWidget(Reference* ref, SymbolDataType::SymbolType type, ReferenceViewer *parent = nullptr);
    ~SymbolReferenceWidget() override;
    void selectSearchField() const;
    QList<QHeaderView*> headers();
    bool isModelLoaded() const;

public slots:
    void updateSelectedSymbol(const QItemSelection &selected, const QItemSelection &deselected);
    void updateSelectedReference(const QItemSelection &selected, const QItemSelection &deselected);
    void expandResetModel();
    void resetModel();
    void initModel();
    void initModel(gams::studio::reference::Reference *ref);
    void jumpToFile(const QModelIndex &index);
    void jumpToReferenceItem(const QModelIndex &index);
    void updateSymbolSelection();
    void resizeColumnToContents();
    void showContextMenu(QPoint p);
    void selectSymbolReference(const gams::studio::reference::ReferenceItem &item);
    gams::studio::reference::ReferenceItem currentReferenceItem();

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    Ui::SymbolReferenceWidget *ui;
    QMenu mContextMenu;

    SymbolTableModel* mSymbolTableModel;
    ReferenceTreeModel* mReferenceTreeModel;

    Reference* mReference;
    SymbolDataType::SymbolType mType;

    ReferenceViewer* mReferenceViewer;

    SymbolId mCurrentSymbolID = -1;
    QString mCurrentSymbolSelection = "";
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // SYMBOLREFERENCEWIDGET_H
