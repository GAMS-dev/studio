/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "tabbarstyle.h"
#include "logger.h"
#include <QStyleOptionTab>
#include <QPainter>

gams::studio::TabBarStyle::TabBarStyle(QStyle *style) : QProxyStyle(style)
{}

QSize gams::studio::TabBarStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    QSize res = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (const QStyleOptionTabV4 *tab = qstyleoption_cast<const QStyleOptionTabV4 *>(option)) {
        if (isBold((tab->tabIndex))) {
            QFont f = widget->font();
            f.setBold(true);
            int diff = QFontMetrics(f).horizontalAdvance(tab->text) - tab->fontMetrics.horizontalAdvance(tab->text);

            res.setWidth(int(res.width() + diff));
        }
    }
    return res;
}

void dumpPalette(QPalette &pal)
{
    QList<int> codes { QPalette::WindowText, QPalette::Button, QPalette::Light, QPalette::Midlight, QPalette::Dark,
                QPalette::Mid, QPalette::Text, QPalette::BrightText, QPalette::ButtonText, QPalette::Base,
                QPalette::Window, QPalette::Shadow, QPalette::Highlight, QPalette::HighlightedText,
                QPalette::Link, QPalette::LinkVisited, QPalette::AlternateBase};
    QStringList names {"WindowText", "Button", "Light", "Midlight", "Dark", "Mid",
                "Text", "BrightText", "ButtonText", "Base", "Window", "Shadow",
                "Highlight", "HighlightedText",
                "Link", "LinkVisited",
                "AlternateBase"};
    for (int i = 0; i < codes.size(); ++i) {
        DEB() << names.at(i) << "  " << pal.color(QPalette::Normal, QPalette::ColorRole(codes.at(i))).name();
    }
}

void gams::studio::TabBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option,
                                            QPainter *painter, const QWidget *widget) const
{
    if (element == CE_TabBarTabLabel) {
        if (const QStyleOptionTabV4 *tab = qstyleoption_cast<const QStyleOptionTabV4 *>(option)) {
            QStyleOptionTabV4 opt(*tab);
            opt.palette.setColor(QPalette::WindowText, Qt::darkRed);
            QProxyStyle::drawControl(element, &opt, painter, widget);

//            painter->save();
//            if (isBold(opt.tabIndex)) {
//                dumpPalette(opt.palette);
//                QFont f = painter->font();
//                f.setBold(true);
//                painter->setFont(f);
//                painter->setPen(opt.palette.text().color());
//                opt.rect = opt.rect.marginsRemoved(QMargins(12,0,12,0));
//                if (opt.leftButtonSize.width() > 0) opt.rect.setLeft(opt.rect.left() + opt.leftButtonSize.width());
//                if (opt.rightButtonSize.width() > 0) opt.rect.setRight(opt.rect.right() - opt.rightButtonSize.width()-4);
//                QProxyStyle::drawItemText(painter, opt.rect, Qt::AlignVCenter|Qt::AlignLeft, tab->palette, true, tab->text);
//            }
//            painter->restore();
            return;
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

bool gams::studio::TabBarStyle::isBold(int index) const
{
    return index == 2;
}
