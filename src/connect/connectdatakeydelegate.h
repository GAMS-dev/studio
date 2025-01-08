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
#ifndef GAMS_STUDIO_CONNECT_CONNECTDATAKEYDELEGATE_H
#define GAMS_STUDIO_CONNECT_CONNECTDATAKEYDELEGATE_H

#include <QStyledItemDelegate>

#include "connect.h"

namespace gams {
namespace studio {
namespace connect {

class ConnectDataKeyDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ConnectDataKeyDelegate(Connect* connect, QObject *parent = nullptr);
    ~ConnectDataKeyDelegate() override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
    bool eventFilter(QObject * editor, QEvent * event) override;

signals:
    void requestSchemaHelp(const QString &schemaname);
    void requestAppendItem(const QModelIndex &index);
    void requestInsertSchemaItem(const int position, const QModelIndex &index);

    void modificationChanged(bool modifiedState);

private slots:
    void commitAndCloseEditor();

private:
    mutable int mIconWidth;
    mutable int mIconHeight;
    mutable int mIconMargin;

    mutable QMap<QString, QRect> mSchemaHelpPosition;
    mutable QMap<QModelIndex, QRect> mSchemaAppendPosition;

    mutable bool mIsLastEditorClosed;
    mutable QWidget* mLastEditor;

    Connect* mConnect;
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_CONNECT_CONNECTDATAKEYDELEGATE_H
