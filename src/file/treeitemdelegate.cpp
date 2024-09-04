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
#include "treeitemdelegate.h"
#include "projecttreemodel.h"
#include "theme.h"
#include <QPainter>
#include <QLineEdit>

namespace gams {
namespace studio {

TreeItemDelegate::TreeItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{}

void TreeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid()) return;
    QStyleOptionViewItem opt(option);
    opt.state.setFlag(QStyle::State_Selected, false);
    opt.textElideMode = Qt::ElideMiddle;
    opt.palette.setColor(QPalette::Highlight, Qt::transparent);
    bool isProject = index.model()->data(index, ProjectTreeModel::IsProjectRole).toBool();
    QRect btRect = opt.rect;
    if (isProject) {
        btRect.setLeft(opt.rect.right() - opt.rect.height());
        btRect = btRect.marginsRemoved(QMargins(2,2,2,2));
        opt.rect.setRight(opt.rect.right() - opt.rect.height());
    }
    QStyledItemDelegate::paint(painter, opt, index);
    if (isProject) {
        bool act = false;
        if (index.row() == 4)
            act = false;
        if (opt.state.testFlag(QStyle::State_MouseOver))
            act = true;
        Theme::icon(":/%1/cog").paint(painter, btRect, Qt::AlignCenter, act ? QIcon::Normal : QIcon::Disabled);
    }
    QString nameExt = index.model()->data(index, ProjectTreeModel::NameExtRole).toString();
    if (!nameExt.isEmpty()) {
        QRect nxRect = opt.rect;
        nxRect.setLeft(nxRect.left() + opt.decorationSize.width() + 6);
        nxRect = nxRect.marginsRemoved(QMargins(4,4,4,4));
        QString name = index.model()->data(index, Qt::EditRole).toString();
        painter->setPen(Qt::gray);
        painter->setFont(qvariant_cast<QFont>(index.model()->data(index, Qt::FontRole)));
        int w = painter->fontMetrics().horizontalAdvance(name + ' ');
        nxRect.setLeft(nxRect.left() + w);
        painter->drawText(nxRect, nameExt);
    }
}

QWidget *TreeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    QLineEdit *ed = new QLineEdit(option.text, parent);
    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Highlight, parent->topLevelWidget()->palette().color(QPalette::Highlight));
    ed->setPalette(pal);
    return ed;
}

void TreeItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    static_cast<QLineEdit*>(editor)->setText(index.data(Qt::EditRole).toString());
}

void TreeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    model->setData(index, static_cast<QLineEdit*>(editor)->text());
}

void TreeItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    QRect rect(option.rect);
    rect.setLeft(rect.left() + option.decorationSize.width() + 6);
    rect.setWidth(option.rect.width() - option.decorationSize.width() - 8);
    editor->setGeometry(rect);
}

} // namespace studio
} // namespace gams
