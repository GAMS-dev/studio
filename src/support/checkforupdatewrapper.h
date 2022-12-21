/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#ifndef CHECKFORUPDATEWRAPPER_H
#define CHECKFORUPDATEWRAPPER_H

#include <QStringList>

struct c4uRec;
typedef struct c4uRec *c4uHandle_t;

namespace gams {
namespace studio {
namespace support {

class CheckForUpdateWrapper
{
public:
    ///
    /// \brief CheckForUpdateWrapper.
    /// \remark Loads the c4u library.
    ///
    CheckForUpdateWrapper();

    ///
    /// \brief CheckForUpdateWrapper.
    /// \remark Frees the c4u library.
    ///
    ~CheckForUpdateWrapper();

    ///
    /// \brief Checks if the wrapper state is valid.
    /// \return <c>true</c> if the c4u library has been sucessfully
    ///         loaded and the libary version matches the required
    ///         version; otherwise <c>false</c>.
    ///
    bool isValid() const;

    ///
    /// \brief Get c4u library messages.
    /// \return c4u library messages as one string,
    ///         which is joined by <c>"\\n"</c>.
    ///
    QString message();

    ///
    /// \brief Clear all c4u library message.
    /// \remark Use this function to make a instance of this class reusable.
    ///
    void clearMessages();

    ///
    /// \brief Check for GAMS Sutdio and distribution updates.
    /// \return The update check messages.
    /// \remark This function connects to <c>gams.com</c>.
    ///
    QString checkForUpdate();

    ///
    /// \brief Check for GAMS distribution updates.
    /// \return The update check messages.
    /// \remark This function connects to <c>gams.com</c>.
    ///
    QString checkForUpdateShort();

    ///
    /// \brief Get current GAMS distribution version as integer.
    /// \return Latest version integer, like <c>2510</c>.
    ///         <c>-1</c> in case of an error.
    ///
    int currentDistribVersion();

    ///
    /// \brief Get current GAMS distribution version as short string.
    /// \return Short version string, like <c>"25.1"</c> (MAJOR.MINOR).
    ///         <c>""</c> in case of an error.
    ///
    QString currentDistribVersionShort();

    ///
    /// \brief Get latest GAMS distribution version as integer.
    /// \return Latest version integer, like <c>2510</c>.
    /// \remark This function connects to <c>gams.com</c>.
    ///         <c>-1</c> in case of an error.
    ///
    int lastDistribVersion();

    ///
    /// \brief Get latest GAMS distribution version as short string.
    /// \return Short version string, like <c>"25.1"</c> (MAJOR.MINOR).
    /// \remark This function connects to <c>gams.com</c>.
    ///         <c>""</c> in case of an error.
    ///
    QString lastDistribVersionShort();

    ///
    /// \brief Check if the used GAMS distribution is the latest one.
    /// \return <c>true</c> if it is the latest GAMS distribution; otherwise <c>false</c>.
    /// \remark This function connects to <c>gams.com</c>.
    ///
    bool distribIsLatest();

    ///
    /// \brief Get current GAMS Studio version.
    /// \return GAMS Studio version as <c>int</c>, like <c>120</c>.
    /// \remark Used to check for updates.
    ///
    static int studioVersion();

    ///
    /// \brief Get GAMS Distribution version number.
    /// \return The GAMS Distribution version number as string.
    ///
    static QString distribVersionString();

private:
    char* distribVersionString(char *version, size_t length);
    void getMessages(int &messageIndex, char *buffer);

    static int errorCallback(int count, const char *message);

private:
    bool mValid = true;
    c4uHandle_t mC4U;
    QStringList mMessages;
};

}
}
}

#endif // CHECKFORUPDATEWRAPPER_H
