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
#include "version.h"
#include "settings.h"
#include "commonpaths.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"

#include "networkmanager.h"

#include <iostream>
#include <QMessageBox>
#include <QFileOpenEvent>
#include <QLocalSocket>
#include <QWindow>

namespace gams {
namespace studio {

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    parseCmdArgs();
    QString userName;
#ifdef __unix__
    userName = qgetenv("USER");
#else
    userName = qEnvironmentVariable("USERNAME", QString());
#endif
    mServerName = "com.gams.studio." + userName;
    QPalette pal = palette();
    pal.setColor(QPalette::Disabled, QPalette::Window, QColor(255,255,255));

    connect(&mServer, &QLocalServer::newConnection, this, &Application::newConnection);
    connect(&mDistribValidator, &support::DistributionValidator::newError,
            this, &Application::error);
    connect(&mDistribValidator, &support::DistributionValidator::newWarning,
            this, &Application::warning);
    connect(&mDistribValidator, &support::DistributionValidator::newGamsVersion,
            this, &Application::showGamsUpdateWidget);
}

Application::~Application()
{
    NetworkManager::cleanup();
    Settings::releaseSettings();
}

void Application::init()
{
    CommonPaths::setSystemDir(mCmdParser.gamsDir());
    setOrganizationName(GAMS_ORGANIZATION_STR);
    setOrganizationDomain(GAMS_COMPANYDOMAIN_STR);
    setApplicationName(GAMS_PRODUCTNAME_STR);
    Settings::createSettings(mCmdParser.ignoreSettings(),
                             mCmdParser.resetSettings(),
                             mCmdParser.resetView());
    mMainWindow = std::unique_ptr<MainWindow>(new MainWindow());
    mMainWindow->openFiles(mCmdParser.files());
    if (!mOpenPathOnInit.isEmpty()) {
        triggerOpenFile(mOpenPathOnInit);
        mOpenPathOnInit = QString();
    }

    mDistribValidator.start();
    listen();
}

QString Application::serverName() const
{
    return mServerName;
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

void Application::showExceptionMessage(const QString &title, const QString &message) {
    QMessageBox::critical(nullptr, title, message);
}

bool Application::checkForOtherInstance()
{
    QLocalSocket socket;
    socket.connectToServer(mServerName, QLocalSocket::ReadWrite);

    if(socket.waitForConnected()) {
        QByteArray buffer;
        Q_FOREACH (auto f, mCmdParser.files())
            buffer.append(f.toUtf8() + "\n");
        socket.write(buffer);
        socket.waitForBytesWritten();
        return true;
    }
    return false;
}

void Application::listen()
{
    mServer.removeServer(mServerName);
    mServer.listen(mServerName);
}

void Application::newConnection()
{
    QLocalSocket* socket = mServer.nextPendingConnection();
    connect(socket, &QLocalSocket::readyRead, this, &Application::receiveFileArguments);
}

void Application::receiveFileArguments()
{
    QLocalSocket* socket = static_cast<QLocalSocket*>(QObject::sender());
    mMainWindow->openFiles(QString(socket->readAll()).split("\n", Qt::SkipEmptyParts));
    socket->deleteLater();
    mMainWindow->setForegroundOSCheck();
}

void Application::error(const QString &message)
{
    SysLogLocator::systemLog()->append(message, LogMsgType::Error);
}

void Application::warning(const QString &message)
{
    SysLogLocator::systemLog()->append(message, LogMsgType::Warning);
}

bool Application::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        // this is a macOS only event
        auto* openEvent = static_cast<QFileOpenEvent*>(event);
        if (mMainWindow)
            triggerOpenFile(openEvent->url().path());
        else
            mOpenPathOnInit = openEvent->url().path();
    }
    return QApplication::event(event);
}

void Application::showGamsUpdateWidget(const QString &text)
{
    if (mMainWindow) mMainWindow->showGamsUpdateWidget(text);
}

void Application::parseCmdArgs()
{
    switch(mCmdParser.parseCommandLine())
    {
        case gams::studio::CommandLineOk:
            break;
        case gams::studio::CommandLineError:
            std::cerr << mCmdParser.errorText().toStdString() << std::endl;
            mCmdParser.showHelp();
        case gams::studio::CommandLineVersionRequested:
            mCmdParser.showVersion();
        case gams::studio::CommandLineHelpRequested:
            mCmdParser.showHelp();
    }
}

void Application::triggerOpenFile(QString path)
{
    mMainWindow->openFiles({path});
    for (auto window : allWindows()) {
        if (!window->isVisible())
            continue;
        if (window->windowState() & Qt::WindowMinimized) {
            window->show();
            window->raise();
        }
    }
}

} // namespace studio
} // namespace gams
