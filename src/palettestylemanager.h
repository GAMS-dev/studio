/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#ifndef PALETTESTYLEMANAGER_H
#define PALETTESTYLEMANAGER_H

#include <QObject>
#include <QStyleFactory>

class PaletteStyleManager : public QObject
{
    Q_OBJECT
public:
    enum Platform {
        Windows,
        MacOS,
        Linux,
        Unknown
    };
    // prevent instantiation of PaletteStyleManager
    PaletteStyleManager() = delete;

    static Platform currentPlatform() {
#if defined(__APPLE__)
        return MacOS;
#elif defined(_WIN64)
        return Windows;
#elif defined(__unix__)
        return Linux;
#else
        return Unknown;
#endif
    }

    static QStringList initializePlatformRegistry(Platform platform) {
        switch(platform) {
            case Windows: return QStringList{"windowsvista", "Fusion", "Windows"};
            case MacOS  : return QStringList{"macos", "Fusion", "Windows"};
            case Linux  : return QStringList{"breeze", "oxygen", "adwaita", "Fusion", "Windows"};
            default     : return QStringList{"Fusion", "Windows"};
        }
    }

    static bool isStyleSupported(const QString& styleKey) {
        static const QStringList runtimeAvailableStyles = QStyleFactory::keys();
        return runtimeAvailableStyles.contains(styleKey, Qt::CaseInsensitive);
    }

    static QString nativeStyleKey() {
        switch (currentPlatform()) {
           case Windows : return isStyleSupported("windowsvista") ? "windowsvista" : "Fusion";
           case MacOS   : return isStyleSupported("macos")        ? "macos"        : "Fusion";
           default      : return "Fusion";  // Linux and fallback
        }
    }

    static QString uniformStyleKey() {
        return "Fusion";
    }
};

#endif // PALETTESTYLEMANAGER_H
