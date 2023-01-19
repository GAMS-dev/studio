/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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

namespace gams {
namespace studio {
namespace connect {

class ConnectDataKeyDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ConnectDataKeyDelegate(QObject *parent = nullptr);
    ~ConnectDataKeyDelegate() override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    QWidget* lastEditor() const;
    bool isLastEditorClosed() const;

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
    bool eventFilter(QObject * editor, QEvent * event) override;

signals:
    void requestSchemaHelp(const QString &schemaname);
    void requestAppendItem(const QModelIndex &index);

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
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_CONNECT_CONNECTDATAKEYDELEGATE_H
