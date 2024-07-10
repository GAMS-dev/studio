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
#include "macoscocoabridge.h"

#include <Cocoa/Cocoa.h>
#include <QUrl>

void MacOSCocoaBridge::disableDictationMenuItem(bool flag)
{
    [[NSUserDefaults standardUserDefaults] setBool:flag forKey:@"NSDisabledDictationMenuItem"];
}

void MacOSCocoaBridge::disableCharacterPaletteMenuItem(bool flag)
{
    [[NSUserDefaults standardUserDefaults] setBool:flag forKey:@"NSDisabledCharacterPaletteMenuItem"];
}

void MacOSCocoaBridge::setAllowsAutomaticWindowTabbing(bool flag)
{
    [NSWindow setAllowsAutomaticWindowTabbing: flag];
}

void MacOSCocoaBridge::setFullScreenMenuItemEverywhere(bool flag)
{
    [[NSUserDefaults standardUserDefaults] setBool:flag forKey:@"NSFullScreenMenuItemEverywhere"];
}

bool MacOSCocoaBridge::isDarkMode()
{
    NSString *interfaceStyle = [NSUserDefaults.standardUserDefaults valueForKey:@"AppleInterfaceStyle"];
    return [interfaceStyle isEqualToString:@"Dark"];
}

QString MacOSCocoaBridge::bundlePath()
{
    auto mainBundle = [NSBundle mainBundle];
    if (!mainBundle)
        return QString();
    return QUrl::fromNSURL([mainBundle bundleURL]).toLocalFile();
}
