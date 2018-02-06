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
#include "mainwindow.h"
#include "application.h"
#include "exception.h"
#include "commandlineparser.h"

int main(int argc, char *argv[])
{
    gams::studio::Application a(argc, argv);
    a.setOrganizationName("GAMS");
    a.setOrganizationDomain("www.gams.com");
    a.setApplicationName("Studio");

    gams::studio::CommandLineParser clParser;

    QString errorMessage;
    switch(clParser.parseCommandLine(&errorMessage))
    {
        case gams::studio::CommandLineOk:
            break;
        case gams::studio::CommandLineError:
            fputs(qPrintable(errorMessage), stderr);
            fputs("\n\n", stderr);
            fputs(qPrintable(clParser.helpText()), stderr);
            return 1;
        case gams::studio::CommandLineVersionRequested:
            printf("%s %s\n", qPrintable(QCoreApplication::applicationName()),
                   qPrintable(QCoreApplication::applicationVersion()));
            return 0;
        case gams::studio::CommandLineHelpRequested:
            clParser.showHelp();
            Q_UNREACHABLE();
    }

    try {
        gams::studio::MainWindow w(clParser);
        w.show();
        return a.exec();
    } catch (gams::studio::FatalException &e) {
        gams::studio::Application::showBox(QObject::tr("fatal exception"), e.what());
        return -1;
    } catch (gams::studio::Exception &e) {
        gams::studio::Application::showBox(QObject::tr("error"), e.what());
    } catch (QException &e) {
        gams::studio::Application::showBox(QObject::tr("external exception"), e.what());
        e.raise();
    } catch (std::exception &e) {
        QString title(QObject::tr("standard exception"));
        gams::studio::Application::showBox(title, e.what());
        FATAL() << title << " - " << e.what();
    } catch (...) {
        QString msg(QObject::tr("An exception occured. Due to its unknown type the message can't be shown"));
        gams::studio::Application::showBox(QObject::tr("unknown exception"), msg);
        FATAL() << msg;
    }
    return -2;
}
