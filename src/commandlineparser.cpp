/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "commandlineparser.h"
#include <QDebug>

namespace gams {
namespace studio {

CommandLineParser::CommandLineParser()
    : QCommandLineParser()
{

}

CommandLineParseResult CommandLineParser::parseCommandLine()
{
    const QCommandLineOption helpOption = addHelpOption();
    const QCommandLineOption versionOption = addVersionOption();
    addPositionalArgument("files", "List of files to be opened.", "[files]");
    addOption({"ignore-settings", "Completely ignore settings files."});
    addOption({"reset-settings", "Reset all settings to default."});

    if (!parse(QCoreApplication::arguments()))
        return CommandLineError;
    if (isSet(versionOption))
        return CommandLineVersionRequested;
    if (isSet(helpOption))
        return CommandLineHelpRequested;
    if (isSet("ignore-settings"))
        mIgnoreSettings = true;
    if (isSet("reset-settings"))
        mResetSettings = true;
    mFiles = positionalArguments();

    return CommandLineOk;
}

QStringList CommandLineParser::files() const
{
    return mFiles;
}

bool CommandLineParser::ignoreSettings() const
{
    return mIgnoreSettings;
}

bool CommandLineParser::resetSettings() const
{
    return mResetSettings;
}

} // namespace studio
} // namespace gams
