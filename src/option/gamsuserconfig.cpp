/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "gamsuserconfig.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "commonpaths.h"
#include "exception.h"
#include "common.h"

namespace gams {
namespace studio {
namespace option {

GamsUserConfig::GamsUserConfig(const QString &location) :
    mLocation(location), mAvailable(false)
{
    auto logger = SysLogLocator::systemLog();

    gucSetExitIndicator(0);    // switch of exit() call
    gucSetScreenIndicator(0);
    gucSetErrorCallback(GamsUserConfig::errorCallback);

    char message[GMS_SSSIZE];
    if (!gucCreateD(&mGUCfg,
                    CommonPaths::systemDir().toStdString().c_str(),
                    message, sizeof(message))) {
       logger->append(message, LogMsgType::Error);
       EXCEPT() << QString("Could not initialize from gams system directory: %1.").arg(message);
    }

    gucCreate(&mGUCfg, message,sizeof(message));
    mAvailable = readGAMSUserConfigFile(mLocation);
}

GamsUserConfig::~GamsUserConfig()
{
    if (mGUCfg)  gucFree(&mGUCfg);
}

QList<ConfigItem *> GamsUserConfig::readCommandLineParameters()
{
    QList<ConfigItem *> itemList;
    if (!mAvailable)
        return itemList;

    clearLastErrorMessage();
    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
       int gsec, ival, isset;
       char sval[GMS_SSSIZE], skey[GMS_SSSIZE];

       int rc = gucGetItemKV(mGUCfg, ipos, guciGSection, &isset, &gsec, sval, sizeof(sval)) ;
       if (rc > 1) {
           setLastErrorMessage( QString("Failure to read commandLineParameters from %1").arg(mLocation) );
           break;
       }

       if (1==rc || gsec != CLP)
           continue;

       gucGetItemKV(mGUCfg,ipos,gucsName,&isset,&ival,skey,sizeof(skey));
       gucGetItemKV(mGUCfg,ipos,gucsValue,&isset,&ival,sval,sizeof(sval));
       if (isset) {
           ConfigItem* item = new ConfigItem(QString(skey), QString(sval));
           gucGetItemKV(mGUCfg,ipos,gucsMinVersion,&isset,&ival,skey,sizeof(skey));
           if (isset)
              item->minVersion = QString(skey);
           gucGetItemKV(mGUCfg,ipos,gucsMaxVersion,&isset,&ival,skey,sizeof(skey));
           if (isset)
               item->maxVersion = QString(skey);
           itemList.append(item);
       }
    }
    return itemList;
}

QList<EnvVarConfigItem *> GamsUserConfig::readEnvironmentVariables()
{
    QList<EnvVarConfigItem *> itemList;
    if (!mAvailable)
        return itemList;

    clearLastErrorMessage();
    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
       int gsec, ival, isset;
       char sval[GMS_SSSIZE], skey[GMS_SSSIZE];

       int rc = gucGetItemKV(mGUCfg, ipos, guciGSection, &isset, &gsec, sval, sizeof(sval));
       if (rc > 1) {
           setLastErrorMessage( QString("Failure to read environmentVariables from %1").arg(mLocation) );
           break;
       }

       if (1==rc || gsec != EV)
           continue;

       gucGetItemKV(mGUCfg,ipos,gucsName,&isset,&ival,skey,sizeof(skey));
       gucGetItemKV(mGUCfg,ipos,gucsValue,&isset,&ival,sval,sizeof(sval));
       if (isset) {
           EnvVarConfigItem* item = new EnvVarConfigItem(QString(skey), QString(sval));
           gucGetItemKV(mGUCfg,ipos,gucsMinVersion,&isset,&ival,skey,sizeof(skey));
           if (isset)  item->minVersion = QString(skey);
           gucGetItemKV(mGUCfg,ipos,gucsMaxVersion,&isset,&ival,skey,sizeof(skey));
           if (isset)  item->maxVersion = QString(skey);
           gucGetItemKV(mGUCfg,ipos,guciPathVariable,&isset,&ival,skey,sizeof(skey));
           if (isset)
               item->pathVariable = (ival ? EnvVarConfigItem::pathDefinition::PATH_DEFINED
                                          : EnvVarConfigItem::pathDefinition::NO_PATH_DEFINED);
           else
               item->pathVariable = EnvVarConfigItem::pathDefinition::NONE;
           itemList.append(item);
       }
    }
    return itemList;
}

void GamsUserConfig::updateCommandLineParameters(const QList<ConfigItem *> &items)
{
    clearLastErrorMessage();
    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
        int gsec,  isset;
        char sval[GMS_SSSIZE];
        int rc = gucGetItemKV(mGUCfg, ipos, guciGSection, &isset, &gsec, sval, sizeof(sval));
        if (rc > 1)
            break;
        if (rc==1)
            continue;
        if (gsec == CLP) {
            gucMakeNoneItem(mGUCfg,ipos);
            continue;
        }
    }

    for (ConfigItem* i : items) {
        int ipos;
        gucStartSectionItem( mGUCfg, gucCommandLineParameters, i->key.toStdString().c_str(), &ipos);
        gucAddItemKV( mGUCfg,gucsValue,0,i->value.toStdString().c_str());
        if (!i->minVersion.isEmpty())
           gucAddItemKV( mGUCfg,gucsMinVersion,0, i->minVersion.toStdString().c_str());
        if (!i->maxVersion.isEmpty())
           gucAddItemKV( mGUCfg, gucsMaxVersion,0, i->maxVersion.toStdString().c_str());
        if (0==gucDoneSectionItem(mGUCfg)) {
            char msg[GMS_SSSIZE];
            if (0==gucGetErrorMessage( mGUCfg, msg, sizeof(msg)))
                setLastErrorMessage(QString(msg));
        }
    }
}

void GamsUserConfig::updateEnvironmentVariables(const QList<EnvVarConfigItem *> &items)
{
    clearLastErrorMessage();
    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
        int gsec,  isset;
        char sval[GMS_SSSIZE];
        int rc = gucGetItemKV(mGUCfg, ipos, guciGSection, &isset, &gsec, sval, sizeof(sval));
        if (rc > 1)
            break;
        if (1==rc)
            continue;
        if (gsec == EV) {
            gucMakeNoneItem(mGUCfg,ipos);
            continue;
        }
    }

    for (EnvVarConfigItem* i : items) {
        int ipos;
        gucStartSectionItem( mGUCfg, gucEnvironmentVariables, i->key.toStdString().c_str(), &ipos);
        gucAddItemKV( mGUCfg,gucsValue,0,i->value.toStdString().c_str());
        if (!i->minVersion.isEmpty())
           gucAddItemKV( mGUCfg,gucsMinVersion,0, i->minVersion.toStdString().c_str());
        if (!i->maxVersion.isEmpty())
            gucAddItemKV( mGUCfg, gucsMaxVersion,0, i->maxVersion.toStdString().c_str());
        // int ret = check if (ret != 0) error report ?? : qDebug() << i->maxVersion << ":ret=" << ret;
        if (i->hasPathVariable())
            gucAddItemKV( mGUCfg,guciPathVariable, i->pathVariable, "");

        if (0==gucDoneSectionItem(mGUCfg)) {
            char msg[GMS_SSSIZE];
            if (0==gucGetErrorMessage( mGUCfg, msg, sizeof(msg)))
                setLastErrorMessage( msg );
        }
    }
}

void GamsUserConfig::writeGamsUserConfigFile(const QString &location)
{
    clearLastErrorMessage();
    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
        int gsec,  isset;
        char sval[GMS_SSSIZE];
        int rc = gucGetItemKV(mGUCfg, ipos, guciGSection, &isset, &gsec, sval, sizeof(sval));
        if (rc > 1)
            break;
        if (1==rc)
            continue;
    }

    gucWriteGAMSConfig(mGUCfg, location.toStdString().c_str());
}

bool GamsUserConfig::reloadGAMSUserConfigFile(const QString &location)
{
    mLocation  = location;
    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
        gucMakeNoneItem(mGUCfg,ipos);
    }

    mAvailable = readGAMSUserConfigFile(mLocation);
    return mAvailable;
}

bool GamsUserConfig::readGAMSUserConfigFile(const QString &location)
{
    if (0!=gucReadGAMSConfig(mGUCfg, location.toStdString().c_str())) {
        char msg[GMS_SSSIZE];
        if (0==gucGetErrorMessage( mGUCfg, msg, sizeof(msg)))
           setLastErrorMessage( msg );
        return false;
    } else {
        return true;
    }
}

void GamsUserConfig::clearLastErrorMessage()
{
    mLastErrorMessage.clear();
}

void GamsUserConfig::setLastErrorMessage(const char *message)
{
    mLastErrorMessage = QString(message);
}

void GamsUserConfig::setLastErrorMessage(const QString &message)
{
    mLastErrorMessage = message;
}

QString GamsUserConfig::getLastErrorMessage() const
{
    return mLastErrorMessage;
}

bool GamsUserConfig::isAvailable() const
{
    return mAvailable;
}

int GamsUserConfig::errorCallback(int count, const char *message)
{
    Q_UNUSED(count)
    auto logger = SysLogLocator::systemLog();
    logger->append(InvalidGAMS, LogMsgType::Error);
    logger->append(message, LogMsgType::Error);
    return 0;
}

} // namespace option
} // namespace studio
} // namespace gams
