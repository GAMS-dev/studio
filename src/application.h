/*
 * This file is part of the GAMS Studio project.
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
#ifndef APPLICATION_H
#define APPLICATION_H

#include "commandlineparser.h"
#include "support/distributionvalidator.h"
#include "mainwindow.h"

#include <QApplication>
#include <QLocalServer>

namespace gams {
namespace studio {

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application() override;

    ///
    /// \brief Get the application MainWindow.
    /// \return MainWindow managed by <c>Application</c> class.
    ///
    QSharedPointer<MainWindow> mainWindow() const;

    bool notify(QObject *object, QEvent *event) override;

    ///
    /// \brief Show a <c>QMessageBox::critical</c> message.
    /// \param title Title of the message.
    /// \param message The exception/error message.
    ///
    static void showExceptionMessage(const QString &title, const QString &message);

    ///
    /// \brief Check if other GAMS Studio instances are running and forward files from the command line that needs to be opened to the running instance.
    /// \return True if another instance is already running, false otherwise.
    ///
    bool checkForOtherInstance();

    ///
    /// \brief Initialize the application
    ///
    void init();

    ///
    /// \brief Get the server name.
    /// \return Returns the server name.
    ///
    QString serverName() const;

    ///
    /// \brief Get skip check for update value.
    /// \return Returns the skip check for update value.
    ///
    bool skipCheckForUpdate() const;

protected:
    ///
    /// \brief Reimplemented QObject::event function.
    /// \param e Event to handle.
    /// \return <c>true</c> on success; otherwise <c>false</c>.
    /// \remark This function if requried for macos, e.g. for file associations.
    ///
    bool event(QEvent *event) override;

private slots:
    void error(const QString &message);

    void warning(const QString &message);

    ///
    /// \brief Bind a new connection for command line argument awareness
    ///
    void newConnection();

    ///
    /// \brief Read command line arguments (files) send from another GAMS Studio instance and open them
    ///
    void receiveFileArguments();

private:
    void parseCmdArgs();
    void triggerOpenFile(const QString &path);

    ///
    /// \brief Start listening
    ///
    void listen();

private:
    QSharedPointer<MainWindow> mMainWindow;
    CommandLineParser mCmdParser;
    support::DistributionValidator mDistribValidator;
    QString mOpenPathOnInit;
    QString mServerName;
    QLocalServer mServer;
};

} // namespace studio
} // namespace gams

#endif // APPLICATION_H
