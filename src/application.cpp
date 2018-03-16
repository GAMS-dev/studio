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

#include <QMessageBox>
#include <QFileOpenEvent>

#include <QDebug>
#include <fstream>
#include "gamspaths.h"

namespace gams {
namespace studio {

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{

}

bool Application::notify(QObject* object, QEvent* event)
{
    try {
        return QApplication::notify(object, event);
    } catch (FatalException &e) {
        Application::showExceptionMessage(tr("fatal exception"), e.what());
        e.raise();
    } catch (Exception &e) {
        Application::showExceptionMessage(tr("error"), e.what());
    } catch (QException &e) {
        Application::showExceptionMessage(tr("external exception"), e.what());
        e.raise();
    } catch (std::exception &e) {
        QString title(tr("standard exception"));
        Application::showExceptionMessage(title, e.what());
        FATAL() << title << " - " << e.what();
    } catch (...) {
        QString msg(tr("An exception occured. Due to its unknown type the message can't be shown"));
        Application::showExceptionMessage(tr("unknown exception"), msg);
        FATAL() << msg;
    }
    return true;
}

void Application::showExceptionMessage(const QString &title, const QString &message) {
    QMessageBox::critical(nullptr, title, message);
}

QString Application::openFile() const
{
    return mOpenFile;
}

bool Application::event(QEvent *event)
{
    std::ofstream fs;
    auto wd = GAMSPaths::defaultWorkingDir();
    wd.append("/lala.txt");
    fs.open(wd.toStdString(), std::ofstream::out | std::ofstream::app);
    if (event->type() == QEvent::FileOpen) {
        auto* openEvent = static_cast<QFileOpenEvent*>(event);
        mOpenFile = openEvent->url().toString();
        fs << "url >> " << mOpenFile.toStdString() << std::endl;
        fs << "sysdir >> " << GAMSPaths::systemDir().toStdString() << std::endl;
    }
    fs.flush();
    fs.close();
    return QApplication::event(event);
}

} // namespace studio
} // namespace gams
