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
#ifndef CRASHREPORT_H
#define CRASHREPORT_H
#include <QString>

namespace gams {
namespace studio {

// Detects whether the previous run ended without a clean exit and immediately
// zips its log (plus the newest dump younger than the lock file) next to the log.
class CrashReport
{
public:
    CrashReport() = delete;

    struct PreviousRun
    {
        bool failed = false; // last run did not exit cleanly
        bool zipped = false; // zipFile was written
        QString dumpFile;    // dump included in the zip, may be empty
        QString zipFile;
    };

    // Call once at startup, after CrashHandler::install() and setSystemDirectory()
    // (gmszip), and BEFORE the log file is renamed or opened. Blocks until gmszip is done.
    static PreviousRun checkPreviousRun(const QString& logFile);
};

} // namespace studio
} // namespace gams

#endif  // CRASHREPORT_H
