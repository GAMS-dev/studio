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
#include "overview.h"

#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include <QWebEnginePage>

namespace gams {
namespace studio {

class MainWindow;

Overview::Overview(QWidget *parent) :
    QWebEngineView(parent)
{
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    this->setSizePolicy(sizePolicy);
}

QWebEngineView *Overview::createWindow(QWebEnginePage::WebWindowType type)
{
   switch (type) {
   case QWebEnginePage::WebBrowserTab: {
       return nullptr;
   }
   case QWebEnginePage::WebBrowserWindow: {
       return nullptr;
   }
   case QWebEnginePage::WebBrowserBackgroundTab: {
       return nullptr;
   }
   case QWebEnginePage::WebDialog: {
       return nullptr;
   }
   }
   return QWebEngineView::createWindow(type);
}

void Overview::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    menu->addAction( this->pageAction(QWebEnginePage::Reload ));
    menu->addAction( this->pageAction(QWebEnginePage::Stop) );
    if (!selectedText().isEmpty()) {
       menu->addSeparator();
       menu->addAction( this->pageAction(QWebEnginePage::Copy) );
    }
    menu->popup(event->globalPos());
}

} // namespace studio
} // namespace gams
