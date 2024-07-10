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
#ifndef MACOSCOCOABRIDGE_H
#define MACOSCOCOABRIDGE_H

#include <QString>

class MacOSCocoaBridge
{
private:
    MacOSCocoaBridge() {}

public:
    static void disableDictationMenuItem(bool flag);
    static void disableCharacterPaletteMenuItem(bool flag);
    static void setAllowsAutomaticWindowTabbing(bool flag);
    static void setFullScreenMenuItemEverywhere(bool flag);
    static bool isDarkMode();

    static QString bundlePath();
};

#endif // MACOSCOCOABRIDGE_H
