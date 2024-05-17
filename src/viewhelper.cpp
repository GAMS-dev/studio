/*
 * This file is part of the GAMS Studio project.
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
#include "viewhelper.h"
#include "editors/abstractedit.h"
#include "file/filemeta.h"
#include "settings.h"
#ifndef __APPLE__
# include "colors/palettemanager.h"
#endif

#ifdef __APPLE__
#include "../platform/macos/macoscocoabridge.h"
#endif

#include <QVariant>

namespace gams {
namespace studio {

ViewHelper::ViewHelper()
{ }

QString ViewHelper::location(QWidget *widget)
{
    return widget->property("location").toString();
}

void ViewHelper::setLocation(QWidget *widget, const QString &location)
{
    widget->setProperty("location", location);
    // if there is an inner edit: set the property additionally
    if (AbstractEdit *ed = toAbstractEdit(widget)) {
        if (ed != widget) ed->setProperty("location", location);
    } else if (TextView* tv = toTextView(widget)) {
       tv->edit()->setProperty("location", location);
    }
}

bool ViewHelper::modified(const QWidget *widget)
{
    return widget->property("modified").toBool();
}

void ViewHelper::setModified(QWidget *widget, bool modified)
{
    widget->setProperty("modified", modified);
    // if there is an inner edit: set the property additionally
    if (AbstractEdit *ed = toAbstractEdit(widget)) {
        if (ed != widget) ed->setProperty("modified", modified);
    } else if (TextView* tv = toTextView(widget)) {
       tv->edit()->setProperty("modified", modified);
    }
}

bool ViewHelper::updateBaseTheme()
{
    int currentTheme = Theme::instance()->activeTheme();
#ifdef __APPLE__
    Theme::instance()->setActiveTheme(MacOSCocoaBridge::isDarkMode() ? 1 : 0);
    if (currentTheme != Theme::instance()->activeTheme()) {
        Settings::settings()->setInt(skEdAppearance, Theme::instance()->activeTheme());
        Settings::settings()->save();
    }
#endif
    return currentTheme != Theme::instance()->activeTheme();
}

///
/// \brief ViewHelper::setAppearance sets and saves the appearance
/// \param appearance
///
void ViewHelper::setAppearance(int appearance)
{
    if (appearance == -1)
        appearance = Settings::settings()->toInt(skEdAppearance);

    Settings::settings()->setInt(skEdAppearance, appearance);
    changeAppearance(appearance);
}

///
/// \brief ViewHelper::changeAppearance sets the appearance without saving it into settings.
/// Useful for previews. ! THIS CHANGE IS NOT PERSISTENT !
/// \param appearance
/// \return
///
void ViewHelper::changeAppearance(int appearance)
{
    if (appearance == -1)
        appearance = Settings::settings()->toInt(skEdAppearance);
    int pickedTheme = appearance;

#ifdef _WIN32
    bool canFollowOS = true; // deactivate follow OS option for linux

    if (canFollowOS && pickedTheme == 0) { // do OS specific things
        QSettings readTheme("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::Registry64Format);
        pickedTheme = readTheme.value("AppsUseLightTheme").toBool() ? 0 : 1;
    } else if (canFollowOS) {
        pickedTheme--; // deduct "Follow OS" option
    }
#endif

#ifndef __APPLE__
    int base = Theme::instance()->baseTheme(pickedTheme);
    PaletteManager::instance()->setPalette(base);
#endif
    Theme::instance()->setActiveTheme(pickedTheme);
}

} // namespace studio
} // namespace gams
