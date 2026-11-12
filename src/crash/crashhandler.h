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
#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H
#include <QString>
#include <QStringList>

namespace gams {
namespace studio {

// Writes minidumps on crashes (Windows). On Linux/macOS the OS handles crashes.
class CrashHandler
{
public:
    CrashHandler() = delete;

    static bool install(const QString& dumpDirectory = QString()); // call once after QApplication
    static bool isInstalled();
    static QString dumpDirectory();
    static bool writeDump(const QString& namePrefix = QStringLiteral("dump")); // keeps running; Windows only
    static QStringList sessionDumps(); // dumps written by writeDump() in this session
};

}
}

#endif // CRASHHANDLER_H
