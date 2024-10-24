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
#include "application.h"
#include "exception.h"
#include "qstylefactory.h"
#include "version.h"
#include "settings.h"
#include "commonpaths.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "logger.h"
#include "networkmanager.h"
#ifndef __APPLE__
# include "colors/palettemanager.h"
#endif

#include <iostream>
#include <QDir>
#include <QMessageBox>
#include <QFileOpenEvent>
#include <QLocalSocket>
#include <QWindow>

namespace gams {
namespace studio {

QtMessageHandler vOriginalLogHandler = nullptr;
QString vCustomLogFile;

void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString message = qFormatLogMessage(type, context, msg);
    static std::FILE *f = std::fopen(vCustomLogFile.toLatin1().data(), "a");
    if (f) {
        std::fprintf(f, "%s\n", qPrintable(message));
        std::fflush(f);
    } else {
        qInstallMessageHandler(vOriginalLogHandler);
        qDebug() << "failed to open log file" << vCustomLogFile;
    }
    if (vOriginalLogHandler)
        vOriginalLogHandler(type, context, msg);
}


Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
#ifdef _WIN32
    QApplication::setStyle(QStyleFactory::create("Fusion"));
#endif
    QLocale *locale = new QLocale(QLocale::English, QLocale::UnitedStates);
    QLocale::setDefault(*locale);
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
}

Application::~Application()
{
    NetworkManager::cleanup();
    Settings::releaseSettings();
#ifndef __APPLE__
    PaletteManager::deleteInstance();
#endif
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
    if (Settings::settings()->toBool(skEnableLog) && mCmdParser.logFile().isEmpty())
        vCustomLogFile = CommonPaths::studioDocumentsDir() + "/studio.log";
    else if (!mCmdParser.logFile().isEmpty() && mCmdParser.logFile() != "-")
        vCustomLogFile = mCmdParser.logFile();

    if (!vCustomLogFile.isEmpty()) {
        if (QFile::exists(vCustomLogFile)) {
            if (QFile::exists(vCustomLogFile + "~"))
                QFile::remove(vCustomLogFile + "~");
            QFile::rename(vCustomLogFile, vCustomLogFile + "~");
        }
        QFileInfo log(vCustomLogFile);
        if (QFile::exists(log.absolutePath())) {
            qDebug() << "log additionally to file" << vCustomLogFile;
            vOriginalLogHandler = qInstallMessageHandler(*logToFile);
        } else
            qDebug() << "Error: Couldn't register log file in missing path " << log.absolutePath();
    }
    QStringList success, failed;
    clearWorkspace(Settings::settings()->toBool(skCleanUpWorkspace),
                   Settings::settings()->toString(skCleanUpWorkspaceFilter).split(",", Qt::SkipEmptyParts),
                   success, failed);

    mMainWindow = QSharedPointer<MainWindow>(new MainWindow());
    connect(mMainWindow.get(), &MainWindow::cleanupWorkspace, this, &Application::clearWorkspaceNow);
    mMainWindow->appendSystemLogInfo("Started: " + QCoreApplication::arguments().join(" "));
    mMainWindow->openFiles(mCmdParser.files());
    if (!mOpenPathOnInit.isEmpty()) {
        triggerOpenFile(mOpenPathOnInit);
        mOpenPathOnInit = QString();
    }
    mDistribValidator.start();
    listen();
    reportCleanupWsState(success, failed);
    DEB() << GAMS_PRODUCTNAME_STR << " " << applicationVersion() << " on " << QSysInfo::prettyProductName() << " "
          << QSysInfo::currentCpuArchitecture() << " build " << QSysInfo::kernelVersion();
    GamsProcess gp;
    DEB() << gp.aboutGAMS().split("\n").first();
}

QString Application::serverName() const
{
    return mServerName;
}

bool Application::skipCheckForUpdate() const
{
    return mCmdParser.skipCheckForUpdate();
}

QSharedPointer<MainWindow> Application::mainWindow() const
{
    return mMainWindow;
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
        const auto files = mCmdParser.files();
        for (const auto &f : files)
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

void Application::clearWorkspaceNow(const QStringList &filters)
{
    QStringList success, failed;
    clearWorkspace(true, filters, success, failed);
    reportCleanupWsState(success, failed);
}

void Application::clearWorkspace(bool active, const QStringList &filters,
                                 QStringList &rmSuccess, QStringList &rmFailed)
{
    if (!active || filters.isEmpty())
        return;
    QStringList patterns;
    for (const auto& filter : filters) {
        patterns << filter.trimmed();
    }
    auto ws = Settings::settings()->toString(skDefaultWorkspace);
    if (ws.isEmpty())
        return;
    QDir dir(ws);
    dir.setNameFilters(patterns);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    auto entries = dir.entryInfoList();
    for (auto entry : entries) {
        if (entry.isDir()) {
            auto str = entry.absoluteFilePath();
            QDir subDir(str);
            if (subDir.removeRecursively())
                rmSuccess << str;
            else
                rmFailed << str;
        } else {
            auto str = entry.fileName();
            if (dir.remove(entry.fileName()))
                rmSuccess << str;
            else
                rmFailed << str;
        }
    }
}

void Application::reportCleanupWsState(const QStringList &rmSuccess, const QStringList &rmFailed)
{
    if (!rmSuccess.isEmpty()) {
        QString message("Successfully deleted directories and files:\n" + rmSuccess.join("\n"));
        SysLogLocator::systemLog()->append(message, LogMsgType::Info);
    }
    if (!rmFailed.isEmpty()) {
        QString message("Failed to delete directories and files:\n" + rmFailed.join("\n"));
        SysLogLocator::systemLog()->append(message, LogMsgType::Error);
    }
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

void Application::parseCmdArgs()
{
    switch(mCmdParser.parseCommandLine()) {
    case gams::studio::CommandLineOk:
        break;
    case gams::studio::CommandLineError:
        std::cerr << mCmdParser.errorText().toStdString() << std::endl;
        mCmdParser.showHelp();
        break;
    case gams::studio::CommandLineVersionRequested:
        mCmdParser.showVersion();
        break;
    case gams::studio::CommandLineHelpRequested:
        mCmdParser.showHelp();
        break;
    }
}

void Application::triggerOpenFile(const QString &path)
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
