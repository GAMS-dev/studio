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
#include "studiosettings.h"

//#include <cstdlib>
#include <iostream>
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
    parseCmdArgs();
    auto* settings = new StudioSettings(mCmdParser.ignoreSettings(), mCmdParser.resetSettings());
    mMainWindow = std::unique_ptr<MainWindow>(new MainWindow(settings));
}

Application::~Application()
{
    //delete mMainWindow;
}

MainWindow* Application::mainWindow() const
{
    return mMainWindow.get();
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

void Application::openAssociatedFiles()
{
    mMainWindow->openFiles(mCmdParser.files());
}

void Application::showExceptionMessage(const QString &title, const QString &message) {
    QMessageBox::critical(nullptr, title, message);
}

bool Application::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        auto* openEvent = static_cast<QFileOpenEvent*>(event);
        mMainWindow->openFile(openEvent->url().path());

        std::ofstream fs;
        auto wd = gams::studio::GAMSPaths::defaultWorkingDir();
        wd.append("/lala.txt");
        fs.open(wd.toStdString(), std::ofstream::out | std::ofstream::app);
        fs << "path >> " << openEvent->url().path().toStdString() << std::endl;
        fs << "sysdir >> " << gams::studio::GAMSPaths::systemDir().toStdString() << std::endl;
        fs.flush();
        fs.close();
    }
    return QApplication::event(event);
}

void Application::parseCmdArgs()
{
    switch(mCmdParser.parseCommandLine())
    {
        case gams::studio::CommandLineOk:
            break;
        case gams::studio::CommandLineError:
            std::cerr << mCmdParser.errorText().toStdString() << std::endl;
            //std::exit(EXIT_FAILURE);
        case gams::studio::CommandLineVersionRequested:
            mCmdParser.showVersion();
        case gams::studio::CommandLineHelpRequested:
            mCmdParser.showHelp();
    }
}

} // namespace studio
} // namespace gams
