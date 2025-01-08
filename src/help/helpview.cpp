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
#include <QWebEnginePage>
#include <QAction>

#include "helpwidget.h"
#include "helpview.h"
#include "mainwindow.h"

namespace gams {
namespace studio {

class MainWindow;

namespace help {

HelpView::HelpView(QWidget *parent) :
    QWebEngineView(parent), mCurrentHovered("")
{
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    this->setSizePolicy(sizePolicy);
}

void HelpView::setPage(QWebEnginePage *page)
{
    QWebEngineView::setPage(page);
}

void HelpView::setCurrentHoveredLink(const QString &url)
{
    mCurrentHovered = url;
}

QWebEngineView *HelpView::createWindow(QWebEnginePage::WebWindowType type)
{
    MainWindow *mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow)
        return nullptr;

   switch (type) {
   case QWebEnginePage::WebBrowserTab: {
       return mainWindow->helpWidget()->createHelpView();
   }
   case QWebEnginePage::WebBrowserWindow: {
       return mainWindow->helpWidget()->createHelpView();
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

void HelpView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    if (mCurrentHovered.isEmpty()) {
       menu->addAction( this->pageAction(QWebEnginePage::Back) );
       menu->addAction( this->pageAction(QWebEnginePage::Forward) );
       menu->addAction( this->pageAction(QWebEnginePage::Reload ));
       menu->addAction( this->pageAction(QWebEnginePage::Stop) );
    } else {
       menu->addAction( this->pageAction(QWebEnginePage::OpenLinkInThisWindow) );
       menu->addSeparator();
       menu->addAction( this->pageAction(QWebEnginePage::CopyLinkToClipboard) );
    }
    if (!selectedText().isEmpty()) {
       menu->addSeparator();
       menu->addAction( this->pageAction(QWebEnginePage::Copy) );
    }
    menu->popup(event->globalPos());
}

} // namespace help
} // namespace studio
} // namespace gams
