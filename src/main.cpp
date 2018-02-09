/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include <iostream>
#include <cstdlib>

#include "mainwindow.h"
#include "application.h"
#include "exception.h"
#include "commandlineparser.h"
#include "studiosettings.h"

using gams::studio::Application;
using gams::studio::MainWindow;
using gams::studio::StudioSettings;

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    app.setOrganizationName("GAMS");
    app.setOrganizationDomain("www.gams.com");
    app.setApplicationName("GAMS Studio");
    app.setApplicationVersion(STUDIO_VERSION);

    gams::studio::CommandLineParser clParser;
    switch(clParser.parseCommandLine())
    {
        case gams::studio::CommandLineOk:
            break;
        case gams::studio::CommandLineError:
            std::cerr << clParser.errorText().toStdString() << std::endl;
            return EXIT_FAILURE;
        case gams::studio::CommandLineVersionRequested:
            clParser.showVersion();
        case gams::studio::CommandLineHelpRequested:
            clParser.showHelp();
    }

    try {
        auto* settings = new StudioSettings(clParser.ignoreSettings(), clParser.resetSettings());
        MainWindow w(settings);
        w.openFiles(clParser.files());
        w.show();
        return app.exec();
    } catch (gams::studio::FatalException &e) {
        Application::showExceptionMessage(QObject::tr("fatal exception"), e.what());
        return -1;
    } catch (gams::studio::Exception &e) {
        Application::showExceptionMessage(QObject::tr("error"), e.what());
    } catch (QException &e) {
        Application::showExceptionMessage(QObject::tr("external exception"), e.what());
        e.raise();
    } catch (std::exception &e) {
        QString title(QObject::tr("standard exception"));
        Application::showExceptionMessage(title, e.what());
        FATAL() << title << " - " << e.what();
    } catch (...) {
        QString msg(QObject::tr("An exception occured. Due to its unknown type the message can't be shown"));
        Application::showExceptionMessage(QObject::tr("unknown exception"), msg);
        FATAL() << msg;
    }
    return -2;
}
