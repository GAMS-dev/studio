/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
//    auto logger = SysLogLocator::systemLog();

//    gucSetExitIndicator(0);    // switch of exit() call
//    gucSetScreenIndicator(0);
//    gucSetErrorCallback(GamsUserConfig::errorCallback);

//    char message[GMS_SSSIZE];
//    if (!gucCreateD(&mGUCfg,
//                    CommonPaths::systemDir().toStdString().c_str(),
//                    message, sizeof(message))) {
//       logger->append(message, LogMsgType::Error);
//       EXCEPT() << QString("Could not initialize from gams system directory: %1.").arg(message);
//    }

//    gucCreate(&mGUCfg, message,sizeof(message));

//    readGAMSUserConfigFile(mLocation);
}

GamsUserConfig::~GamsUserConfig()
{
//    if (mGUCfg)  gucFree(&mGUCfg);
}

QList<ConfigItem *> GamsUserConfig::readCommandLineParameters() const
{
    QList<ConfigItem *> itemList;
//    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
//       int gsec, ival, isset;
//       char sval[GMS_SSSIZE], skey[GMS_SSSIZE];

//       int rc = gucGetItemKV(mGUCfg, ipos, guciGSection, &isset, &gsec, sval, sizeof(sval)) ;
//       if (rc > 1) {
//           SysLogLocator::systemLog()->append(QString("Failure to read commandLineParameters from %1").arg(mLocation),
//                                              LogMsgType::Error);
//           break;
//       }

//       if (1==rc || gsec != CLP)
//           continue;

//       gucGetItemKV(mGUCfg,ipos,gucsName,&isset,&ival,skey,sizeof(skey));
//       gucGetItemKV(mGUCfg,ipos,gucsValue,&isset,&ival,sval,sizeof(sval));
//       if (isset) {
//           ConfigItem* item = new ConfigItem(QString(skey), QString(sval));
//           gucGetItemKV(mGUCfg,ipos,gucsMinVersion,&isset,&ival,skey,sizeof(skey));
//           if (isset)
//              item->minVersion = QString(skey);
//           gucGetItemKV(mGUCfg,ipos,gucsMaxVersion,&isset,&ival,skey,sizeof(skey));
//           if (isset)
//               item->maxVersion = QString(skey);
//           itemList.append(item);
//       }
//    }
    return itemList;
}

QList<EnvVarConfigItem *> GamsUserConfig::readEnvironmentVariables() const
{
    QList<EnvVarConfigItem *> itemList;
//    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
//       int gsec, ival, isset;
//       char sval[GMS_SSSIZE], skey[GMS_SSSIZE];

//       int rc = gucGetItemKV(mGUCfg, ipos, guciGSection, &isset, &gsec, sval, sizeof(sval));
//       if (rc > 1) {
//           SysLogLocator::systemLog()->append(QString("Failure to read environmentVariables from %1").arg(mLocation),
//                                              LogMsgType::Error);
//           break;
//       }

//       if (1==rc || gsec != EV)
//           continue;

//       gucGetItemKV(mGUCfg,ipos,gucsName,&isset,&ival,skey,sizeof(skey));
//       gucGetItemKV(mGUCfg,ipos,gucsValue,&isset,&ival,sval,sizeof(sval));
//       if (isset) {
//           EnvVarConfigItem* item = new EnvVarConfigItem(QString(skey), QString(sval));
//           gucGetItemKV(mGUCfg,ipos,gucsMinVersion,&isset,&ival,skey,sizeof(skey));
//           if (isset)  item->minVersion = QString(skey);
//           gucGetItemKV(mGUCfg,ipos,gucsMaxVersion,&isset,&ival,skey,sizeof(skey));
//           if (isset)  item->maxVersion = QString(skey);
//           gucGetItemKV(mGUCfg,ipos,guciPathVariable,&isset,&ival,skey,sizeof(skey));
//           if (isset)
//               item->pathVariable = (ival ? EnvVarConfigItem::pathDefinition::PATH_DEFINED
//                                          : EnvVarConfigItem::pathDefinition::NO_PATH_DEFINED);
//           else
//               item->pathVariable = EnvVarConfigItem::pathDefinition::NONE;
//           itemList.append(item);
//       }
//    }
    return itemList;
}

void GamsUserConfig::updateCommandLineParameters(const QList<ConfigItem *> &items)
{
//    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
//        int gsec,  isset;
//        char sval[GMS_SSSIZE];
//        int rc = gucGetItemKV(mGUCfg, ipos, guciGSection, &isset, &gsec, sval, sizeof(sval));
//        if (rc > 1)
//            break;
//        if (rc==1)
//            continue;
//        qDebug() << QString("CLP ipos=%1, gsec=%2").arg(ipos).arg(guc_gsection_Name[gsec]);
//        if (gsec == CLP) {
//            gucMakeNoneItem(mGUCfg,ipos);
//            qDebug() << QString("CLP none ipos=%1").arg(ipos);
//            continue;
//        }
//    }

//    for (ConfigItem* i : items) {
//        int ipos;
//        gucStartSectionItem( mGUCfg, gucCommandLineParameters, i->key.toStdString().c_str(), &ipos);
//        qDebug() << QString("CLP update ipos=%1").arg(ipos);
//        gucAddItemKV( mGUCfg,gucsValue,0,i->value.toStdString().c_str());
//        if (!i->minVersion.isEmpty())
//           gucAddItemKV( mGUCfg,gucsMinVersion,0, i->minVersion.toStdString().c_str());
//        if (!i->maxVersion.isEmpty())
//           gucAddItemKV( mGUCfg, gucsMaxVersion,0, i->maxVersion.toStdString().c_str());
//        if (gucDoneSectionItem(mGUCfg)) {
//            char msg[GMS_SSSIZE];
//            if (0==gucGetErrorMessage( mGUCfg, msg, sizeof(msg)))
//                qDebug() << QString("Error: %1").arg(msg);
//        }
//    }
}

void GamsUserConfig::updateEnvironmentVariables(const QList<EnvVarConfigItem *> &items)
{
//    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
//        int gsec,  isset;
//        char sval[GMS_SSSIZE];
//        int rc = gucGetItemKV(mGUCfg, ipos, guciGSection, &isset, &gsec, sval, sizeof(sval));
//        if (rc > 1)
//            break;
//        if (1==rc)
//            continue;
//        qDebug() << QString("EV ipos=%1, gsec=%2").arg(ipos).arg( guc_gsection_Name[gsec]);
//        if (gsec == EV) {
//            gucMakeNoneItem(mGUCfg,ipos);
//            qDebug() << QString("EV none ipos=%1").arg(ipos);
//            continue;
//        }
//    }

//    for (EnvVarConfigItem* i : items) {
//        int ipos;
//        gucStartSectionItem( mGUCfg, gucEnvironmentVariables, i->key.toStdString().c_str(), &ipos);
//        qDebug() << QString("EV update ipos=%1").arg(ipos);
//        gucAddItemKV( mGUCfg,gucsValue,0,i->value.toStdString().c_str());
//        if (!i->minVersion.isEmpty())
//           gucAddItemKV( mGUCfg,gucsMinVersion,0, i->minVersion.toStdString().c_str());
//        if (!i->maxVersion.isEmpty())
//            gucAddItemKV( mGUCfg, gucsMaxVersion,0, i->maxVersion.toStdString().c_str());
//        // int ret =
//           // TODO (JP) check if (ret != 0) error report ?? : qDebug() << i->maxVersion << ":ret=" << ret;
//        if (i->hasPathVariable())
//            gucAddItemKV( mGUCfg,guciPathVariable, i->pathVariable, "");

//        if (gucDoneSectionItem(mGUCfg)) {
//            char msg[GMS_SSSIZE];
//            if (0==gucGetErrorMessage( mGUCfg, msg, sizeof(msg)))
//               qDebug() << QString("Error: %1").arg(msg);
//        }
//    }
}

void GamsUserConfig::writeGamsUserConfigFile(const QString &location)
{
//    qDebug() << "writing : " << location.toStdString().c_str();
//    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
//        int gsec,  isset;
//        char sval[GMS_SSSIZE];
//        int rc = gucGetItemKV(mGUCfg, ipos, guciGSection, &isset, &gsec, sval, sizeof(sval));
//        if (rc > 1)
//            break;
//        if (1==rc)
//            continue;
//        qDebug() << QString("EV ipos=%1, gsec=%2").arg(ipos).arg( guc_gsection_Name[gsec]);
//    }

//    gucWriteGAMSConfig(mGUCfg, location.toStdString().c_str());
}

bool GamsUserConfig::reloadGAMSUserConfigFile(const QString &location)
{
//    for (int ipos=0; ipos<gucGetItemCount(mGUCfg); ipos++) {
//        gucMakeNoneItem(mGUCfg,ipos);
//    }

//    mLocation  = location;
//    readGAMSUserConfigFile(mLocation);
//    // TODO(JP)
    return true;
}

bool GamsUserConfig::readGAMSUserConfigFile(const QString &location)
{
//    auto logger = SysLogLocator::systemLog();
//    char message[GMS_SSSIZE];
//    mLocation = location;
//    if (!gucReadGAMSConfig(mGUCfg, location.toStdString().c_str())) {
//        gucGetErrorMessage(mGUCfg, message, sizeof(message));
//        logger->append(message, LogMsgType::Error);
//        mAvailable = false;
//    } else {
//        mAvailable = true;
//    }
//    return mAvailable;
    return false;
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
