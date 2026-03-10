/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "support/distributionvalidator.h"
#include "exception.h"
#include "qstylefactory.h"
#include "version.h"
#include "settings.h"
#include "commonpaths.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "logger.h"
#include "networkmanager.h"
#include "settingsdialog.h"
#include "msgbox.h"
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

QString sdsToString(SysDirSelector sds)
{
    switch (sds) {
    case sdsManual:
        return "Using user defined GAMS system directory";
    case sdsLocal:
        return "Using local parent GAMS system directory";
    case sdsSystem:
        return "Using GAMS system directory from PATH environment";
    case sdsMac:
        return "Using GAMS system directory from macOS installation";
    }
    return "<Wrong code for SysDirSelector>";
}



Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
#ifdef _WIN64
    QApplication::setStyle(QStyleFactory::create("Fusion"));
#endif
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
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
}

Application::~Application()
{
    NetworkManager::cleanup();
    Settings::releaseSettings();
#ifndef __APPLE__
    PaletteManager::deleteInstance();
#endif
    delete mDistribValidator;
}

bool Application::init()
{
    if (Settings::settings()) {
        DEB() << "Application::init() must not be called twice";
        return true;
    }
    setOrganizationName(GAMS_ORGANIZATION_STR);
    setOrganizationDomain(GAMS_COMPANYDOMAIN_STR);
    setApplicationName(GAMS_PRODUCTNAME_STR);
    Settings::createSettings(mCmdParser.ignoreSettings(),
                             mCmdParser.resetSettings(),
                             mCmdParser.resetView());

    // Init Studio log file
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
            DEB() << "log additionally to file " << QDir::toNativeSeparators(vCustomLogFile);
            vOriginalLogHandler = qInstallMessageHandler(*logToFile);
        } else
            DEB() << "Error: Couldn't register log file in missing path " << log.absolutePath();
    }

    // Set and check GAMS system directory
    QString sysDirMessage;
    SysDirSelector sds = setSystemDirectory(sysDirMessage);
    if (!check4Libs())
        return false;
    DEB() << sdsToString(sds);

    Settings::settings()->setBool(skSupressWebEngine, !mCmdParser.activeHelpView());
    initEnvironment();

    mDistribValidator = new support::DistributionValidator();
    connect(mDistribValidator, &support::DistributionValidator::newError, this, &Application::error);
    connect(mDistribValidator, &support::DistributionValidator::newWarning, this, &Application::warning);
    connect(mDistribValidator, &support::DistributionValidator::foundGamsVersion, this, &Application::updateHighestGamsVersion);

    mMainWindow = QSharedPointer<MainWindow>(new MainWindow());
    if (Settings::settings()->toBool(skCleanUpWorkspace)) {
        auto list = mMainWindow->settingsDialog()->cleanupWorkspaces(true);
        if (!list.isEmpty()) {
            QString msg = QString("Remove %1 files?\n\nThe automatic cleanup would affect %1 files in the chosen directories according to the setting.").arg(list.size());
            QString msgDetail(list.join("\n"));
            int choice = MsgBox::question("Clean Workspace on startup", msg, "", msgDetail, mMainWindow.get(), "Ok", "Cancel");
            if (choice == 0)
                mMainWindow->settingsDialog()->cleanupWorkspaces(false);
        }
    }
    mMainWindow->appendSystemLogInfo("Started: " + QCoreApplication::arguments().join(" "));
    if (!sysDirMessage.isEmpty())
        mMainWindow->appendSystemLogWarning(sysDirMessage);
    mMainWindow->appendSystemLogInfo("GAMS System Directory: " + QDir::toNativeSeparators(CommonPaths::systemDir()));
    mMainWindow->openFiles(mCmdParser.files());
    if (!mOpenPathOnInit.isEmpty()) {
        triggerOpenFile(mOpenPathOnInit);
        mOpenPathOnInit = QString();
    }
    mDistribValidator->start();
    listen();
    DEB() << GAMS_PRODUCTNAME_STR << " " << applicationVersion() << " on " << QSysInfo::prettyProductName() << " "
          << QSysInfo::currentCpuArchitecture() << " build " << QSysInfo::kernelVersion();
    return true;
}

void Application::initEnvironment()
{
    QString gamsDir = QDir::toNativeSeparators(CommonPaths::systemDir());
    if (gamsDir.isEmpty())
        DEB() << "ERROR: GAMS could not be found. Please check your installation of GAMS and GAMS Studio.";

    QByteArray gamsArr = (gamsDir + QDir::listSeparator() + gamsDir + QDir::separator() + "gbin").toLatin1();

    QByteArray curPath = qgetenv("PATH");
    qputenv("PATH", gamsArr + (curPath.isEmpty()? QByteArray() : QDir::listSeparator().toLatin1() + curPath));

#ifndef _WIN64
    curPath = qgetenv("LD_LIBRARY_PATH");
    qputenv("LD_LIBRARY_PATH", gamsArr + (curPath.isEmpty()? QByteArray() : QDir::listSeparator().toLatin1() + curPath));
#endif
#ifdef __APPLE__
    curPath = qgetenv("DYLD_LIBRARY_PATH");
    qputenv("DYLD_LIBRARY_PATH", gamsArr + (curPath.isEmpty()? QByteArray() : QDir::listSeparator().toLatin1() + curPath));
#endif
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

SysDirSelector Application::setSystemDirectory(QString &sysDirMessage)
{
    QString path;
    SysDirSelector sds = sdsManual;
    if (!mCmdParser.gamsDir().isEmpty())
        path = mCmdParser.gamsDir();
    else if (!Settings::settings()->toString(skSystemDirectory).isEmpty())
        path = Settings::settings()->toString(skSystemDirectory);

    if (path.isEmpty()) {
        sds = CommonPaths::setSystemDir();
        return sds;
    }
    QDir dir(path);
    QFileInfo fi(path+"/gamsstmp.txt");
    if (dir.exists() && fi.exists()) {
        sds = CommonPaths::setSystemDir(path);
        Settings::settings()->setString(skSystemDirectory, path);
    } else {
        Settings::settings()->setString(skSystemDirectory, "");
        sds = CommonPaths::setSystemDir();
        if (CommonPaths::systemDir() != path)
            sysDirMessage = "Missing user selected GAMS system directory: " + QDir::toNativeSeparators(path);
    }
    return sds;
}

bool Application::check4Libs()
{
    QStringList miss;
    const QStringList libs = {"joatdclib64", "gdxcclib64", "guccclib64", "optdclib64"};
    QString path = CommonPaths::systemDir() + "/";
#ifdef _WIN64
    for (const QString &lib : libs) {
        QString full = path + lib + ".dll";
        if (!QFile::exists(full))
            miss << QDir::toNativeSeparators(full);
    }
#elif defined(__unix__)
    for (const QString &lib : libs) {
        QString full = path + "lib" + lib + ".so";
        if (!QFileInfo::exists(full))
            miss << full;
    }
#else // macOS
    for (const QString &lib : libs) {
        QString full = path + "lib" + lib + ".dylib";
        if (!QFileInfo::exists(full))
            miss << full;
    }
#endif
    if (!miss.isEmpty()) {
        bool isGams = QFile::exists(path+"/gamsstmp.txt");
        QString title(isGams ? "Incomplete GAMS installation" : "GAMS not found");
        DEB() << title;
        QString missMessage = isGams ? ("Missing GAMS libraries:\n - " + miss.join("\n - ")) : "";
        if (isGams) DEB() << missMessage;
        QMessageBox::warning(nullptr, title, "Please select a valid GAMS installation using the "
                                             "command line parameter \n --gams-dir <path>\n or reinstall GAMS.\n\n"
                                             "Current GAMS directory:\n" + QDir::toNativeSeparators(path)
                                                 + (isGams ? ("\n\n" + missMessage) : ""));
        return false;
    }
    return true;
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

int Application::stringToVersion(const QString &version)
{
    int res = 0;
    const QStringList parts = version.split('.');
    if (parts.size() != 3)
        return -1;
    for (const QString &part : parts) {
        bool ok;
        int nr = part.toInt(&ok);
        if (!ok)
            return -2;
        if (res && nr >= 100)   // report error if subversion or buildnr is > 100
            return -3;
        res = res * 100 + nr;
    }
    return res;
}

void Application::updateHighestGamsVersion(const QString &version)
{
    int currVer = stringToVersion(version);
    int highVer = stringToVersion(Settings::settings()->toString(skHighestGamsUsed));
    if (currVer < 0) {
        DEB() << "GAMS version invalid: '" << version << "'";
        return;
    }
    if (currVer > highVer) {
        Settings::settings()->setString(skHighestGamsUsed, version);
        highVer = currVer;
    }
#ifdef __APPLE__
    QString vPath = CommonPaths::latestGamsDir();
#else
    QString vPath = QDir(qApp->applicationDirPath()+"/..").canonicalPath();
#endif
    QFile vFile(vPath + "/gamsstmp.txt");
    if (vFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::ExistingOnly)) {
        // Peek for gamsstmp.txt in "local" or "mac:highest" gams path to get installed version
        QStringList parts = QString(vFile.readLine()).split(' ');
        vFile.close();
        int peekVer = parts.size() > 1 ? stringToVersion(parts.at(1)) : -4;
        if (peekVer > highVer) {
            if (!Settings::settings()->toString(skSystemDirectory).isEmpty())
                mMainWindow->appendSystemLogWarning("A fixed GAMS version is selected in the settings, "
                                                    "but a later GAMS is found here:\n" + vPath);
            // Settings::settings()->setString(skHighestGamsUsed, parts.at(1));
        }
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
        std::cerr << mCmdParser.errorText().toStdString() << "\n";
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
    auto windows = allWindows();
    for (auto window : std::as_const(windows)) {
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
