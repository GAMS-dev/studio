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
#ifndef APPLICATION_H
#define APPLICATION_H

#include <QtWidgets>

namespace gams {
namespace studio {

class Application : public QApplication
{
public:
    Application(int &argc, char **argv);

    bool notify(QObject *object, QEvent *event) override;

    ///
    /// \brief Show a <c>QMessageBox::critical</c> message.
    /// \param title Title of the message.
    /// \param message The exception/error message.
    ///
    static void showExceptionMessage(const QString &title, const QString &message);

protected:
    ///
    /// \brief Reimplemented QObject::event function.
    /// \param e Event to handle.
    /// \return <c>true</c> on success; otherwise <c>false</c>.
    /// \remark This function if requried for macos, e.g. for file associations.
    ///
    bool event(QEvent *e) override;
};

} // namespace studio
} // namespace gams

#endif // APPLICATION_H
