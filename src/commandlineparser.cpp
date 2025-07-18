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
#include "commandlineparser.h"
#include "commonpaths.h"

namespace gams {
namespace studio {

QString CommandLineParser::NetLog = "";

CommandLineParser::CommandLineParser()
    : QCommandLineParser()
{

}

CommandLineParseResult CommandLineParser::parseCommandLine()
{
    addPositionalArgument("files", "List of files to be opened.", "[files]");
    QStringList helpOpt;
#ifdef _WIN64
    helpOpt << "?";
#endif
    helpOpt << "h" << "help";
    addOption({helpOpt, "Displays help on commandline options."});
    const QCommandLineOption versionOption = addVersionOption();
    addOption({"ignore-settings", "Ignore settings files for loading and saving."});
    addOption({"reset-settings", "Reset all settings including views to default."});
    addOption({"reset-view", "Reset views and window positions only."});
    addOption({"gams-dir", "Set the GAMS system directory", "path"});
    addOption({"log", "Set '$HOME/Documents/GAMS/Studio/studio.log' for Studio system log"});
    addOption({"no-log", "Turns off Studio system log"});
    addOption({"log-file", "Set a log file for Studio system log", "file"});
    addOption({"skip-check-for-update", "Skip all online check for update actions"});
    addOption({"integrated-help", "Switches the integrated help on/off (default: on)", "on/off"});
    addOption({"network-log", "Write the network log for the tools used, like C4U and gamsgetkey", "log file path"});

    if (!parse(QCoreApplication::arguments()))
        return CommandLineError;
    if (isSet(versionOption))
        return CommandLineVersionRequested;
    if (isSet("help"))
        return CommandLineHelpRequested;
    if (isSet("ignore-settings"))
        mIgnoreSettings = true;
    if (isSet("reset-settings"))
        mResetSettings = true;
    if (isSet("reset-view"))
        mResetView = true;
    if (isSet("gams-dir"))
        mGamsDir = this->value("gams-dir");
    if (isSet("log"))
        mLogFile = CommonPaths::studioDocumentsDir() + "/studio.log";
    if (isSet("no-log"))
        mLogFile = "-";
    if (isSet("log-file"))
        mLogFile = this->value("log-file");
    if (isSet("skip-check-for-update"))
        mSkipCheckForUpdate = true;
    if (isSet("integrated-help")) {
        mActiveHelpView = value("integrated-help").compare("on", Qt::CaseInsensitive) == 0 ? 1 :
                          value("integrated-help").compare("off", Qt::CaseInsensitive) == 0 ? 0 : -1;
        if (mActiveHelpView < 0)
            return CommandLineError;
    }
    if (isSet("network-log"))
        NetLog = this->value("network-log");
    mFiles = getFileArgs();

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

bool CommandLineParser::resetView() const
{
    return mResetView;
}

bool CommandLineParser::skipCheckForUpdate() const
{
    return mSkipCheckForUpdate;
}

QString CommandLineParser::gamsDir() const
{
    return mGamsDir;
}

int CommandLineParser::activeHelpView()
{
    // Switch the HelpView (QWebEngine) on(1) off(0) unchanged(-1)
    return mActiveHelpView;
}

QString CommandLineParser::logFile() const
{
    return mLogFile;
}

QString CommandLineParser::c4uLog()
{
    return NetLog;
}

inline QStringList CommandLineParser::getFileArgs()
{
    QStringList absoluteFilePaths;
    const auto args = positionalArguments();
    for (const auto &file : args)
        absoluteFilePaths << CommonPaths::absolutFilePath(file);
    return absoluteFilePaths;
}

} // namespace studio
} // namespace gams
