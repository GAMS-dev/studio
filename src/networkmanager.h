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
#ifndef GAMS_STUDIO_NETWORKMANAGER_H
#define GAMS_STUDIO_NETWORKMANAGER_H

#include <QNetworkReply>
#include "exception.h"

namespace gams {
namespace studio {

class NetworkManager
{
    NetworkManager();
    static QNetworkAccessManager *mNetworkManager;
    static QNetworkAccessManager *mOpenNetworkManager;
    static bool mLock;
public:
    ///
    /// manager: manager to be used for all purposes except self-signed ssl-certificates (see
    ///
    static QNetworkAccessManager *manager() {
        if (mLock)
            EXCEPT() << "NetworkManager already deleted";
        if (!mNetworkManager) {
            mNetworkManager = new QNetworkAccessManager();
        }
        return mNetworkManager;
    }

    ///
    /// managerSelfCert: manager to be used for self-signed ssl-certificates. The peer mode of the  *QSslCertification*
    /// should be set to *QSslSocket::VerifyNone* on the first connection
    ///
    static QNetworkAccessManager *managerSelfCert() {
        if (mLock)
            EXCEPT() << "NetworkManager already deleted";
        if (!mOpenNetworkManager) {
            mOpenNetworkManager = new QNetworkAccessManager();
        }
        return mOpenNetworkManager;
    }

    ///
    /// cleanup: singleton-like: after cleanup the recreation of the manager is locked
    ///
    static void cleanup() {
        if (mNetworkManager) {
            delete mNetworkManager;
            mNetworkManager = nullptr;
        }
        if (mOpenNetworkManager) {
            delete mOpenNetworkManager;
            mOpenNetworkManager = nullptr;
        }
        mLock = true;
    }
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NETWORKMANAGER_H
