/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef PARAMCFGCOMPLETERDELEGATE_H
#define PARAMCFGCOMPLETERDELEGATE_H

#include "optioncompleterdelegate.h"

namespace gams {
namespace studio {
namespace option {

class EnvVarCfgCompleterDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    EnvVarCfgCompleterDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    QModelIndex currentEditedIndex() const;

protected:
    bool eventFilter(QObject * editor, QEvent * event) override;

signals:
    void currentEditedIndexChanged(const QModelIndex &index) const;

private slots:
    void commitAndCloseEditor();
    void updateCurrentEditedIndex(const QModelIndex &index);

private:
    QModelIndex mCurrentEditedIndex;

    mutable bool mIsLastEditorClosed;
    mutable QWidget* mLastEditor;

};

} // namespace option
} // namespace studio
} // namespace gams

#endif // PARAMCFGCOMPLETERDELEGATE_H
