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
#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QCommandLineParser>

namespace gams {
namespace studio {

enum CommandLineParseResult
{
    CommandLineOk,
    CommandLineError,
    CommandLineVersionRequested,
    CommandLineHelpRequested,
};

class CommandLineParser  : public QCommandLineParser
{
public:
    CommandLineParser();
    CommandLineParseResult parseCommandLine();
    QStringList files() const;
    bool ignoreSettings() const;
    bool resetSettings() const;
    bool resetView() const;
    bool skipCheckForUpdate() const;
    QString gamsDir() const;

    QString logFile() const;

private:
    inline QStringList getFileArgs();

private:
    QStringList mFiles;
    bool mIgnoreSettings = false;
    bool mResetSettings = false;
    bool mResetView = false;
    bool mSkipCheckForUpdate = false;
    QString mGamsDir = QString();
    QString mLogFile = QString();
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEPARSER_H
