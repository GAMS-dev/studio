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
#ifndef GAMS_STUDIO_CONNECT_CONNECTACTIONDELEGATE_H
#define GAMS_STUDIO_CONNECT_CONNECTACTIONDELEGATE_H

#include <QStyledItemDelegate>

namespace gams {
namespace studio {
namespace connect {

class ConnectDataValueDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ConnectDataValueDelegate(QObject *parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

protected:
    virtual bool eventFilter(QObject * editor, QEvent * event) override;

signals:
    void modificationChanged(bool modifiedState);

private slots:
    void commitAndCloseEditor();
    void commitAndCloseEditorWithCompleter();

private:
    mutable QString mOriginalText;
    mutable QString mLastText;
    mutable bool mIsCompleter;
    mutable bool mIsLastEditorClosed;
    mutable QWidget* mLastEditor;
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_CONNECT_CONNECTACTIONDELEGATE_H
