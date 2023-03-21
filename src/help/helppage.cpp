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
#include <QDesktopServices>

#include "helppage.h"

namespace gams {
namespace studio {
namespace help {

HelpPage::HelpPage(QObject *parent) : QWebEnginePage(parent)
{
}

bool HelpPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (url.path().toLower().endsWith(".pdf")) {
        QDesktopServices::openUrl(url);
        return true;
    }
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

} // namespace help
} // namespace studio
} // namespace gams
