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
#include "application.h"
#include "exception.h"
#include "version.h"

#include <QSystemSemaphore>

using gams::studio::Application;

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setApplicationVersion(STUDIO_VERSION);

    Application app(argc, argv);
    QSystemSemaphore sem(app.serverName(), 1, QSystemSemaphore::Open);
    sem.acquire();
    if (app.checkForOtherInstance()) {
        sem.release();
        return EXIT_SUCCESS; // terminate since another instance of studio is already running
    }
    app.init();
    sem.release();
    app.setOrganizationName(GAMS_ORGANIZATION_STR);
    app.setOrganizationDomain(GAMS_COMPANYDOMAIN_STR);
    app.setApplicationName(GAMS_PRODUCTNAME_STR);

    try {
        app.mainWindow()->show();
        app.openAssociatedFiles();
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
