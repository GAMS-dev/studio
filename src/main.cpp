/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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

#include <QSystemSemaphore>

using gams::studio::Application;

bool prepareScaling()
{
    bool res = false;
    QString studioScale = qgetenv("GAMS_STUDIO_SCALE_FACTOR");
    QString studioScalePerMonitor = qgetenv("GAMS_STUDIO_SCREEN_SCALE_FACTORS");
    if (qEnvironmentVariableIsEmpty("QT_SCALE_FACTOR") && !studioScale.isEmpty()) {
        qputenv("QT_SCALE_FACTOR", studioScale.toUtf8());
        res = true;
    }
    if (qEnvironmentVariableIsEmpty("QT_SCREEN_SCALE_FACTORS") && !studioScalePerMonitor.isEmpty()) {
        qputenv("QT_SCREEN_SCALE_FACTORS", studioScalePerMonitor.toUtf8());
        res = true;
    }
    return res;
}

int main(int argc, char *argv[])
{
#if !defined(__APPLE__) && defined(__unix__)
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu --no-sandbox");
#endif
    QApplication::setApplicationVersion(STUDIO_VERSION);
    // to temporarily add additional information enable the following line
//    qSetMessagePattern("[%{function}:%{line}]  %{message}");

    if (!prepareScaling())
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // if we manage do get real highDPI icons into the system this may help (currently it scales up lo-res icons)
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    Application app(argc, argv);

    QSystemSemaphore sem(app.serverName(), 1, QSystemSemaphore::Open);
    sem.acquire();
    if (app.checkForOtherInstance()) {
        sem.release();
        return EXIT_SUCCESS; // terminate since another instance of studio is already running
    }
    app.init();
    sem.release();

    try {
        app.mainWindow()->show();
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
