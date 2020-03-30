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
    mLocation(location)
{
    auto logger = SysLogLocator::systemLog();

//    char message[GMS_SSSIZE];
//    if (!ymlCreateD(&mGUCfg,
//                    CommonPaths::systemDir().toStdString().c_str(),
//                    message, sizeof(message))) {
//       logger->append(message, LogMsgType::Error);
//       EXCEPT() << QString("Could not initialize from gams system directory: %1.").arg(message);
//    }

//    ymlCreate(&mGUCfg, message,sizeof(message));
//    if (!ymlReadGAMSConfig(mGUCfg,  mLocation.toStdString().c_str())) {
//        logger->append(message, LogMsgType::Error);
//    }
}

GamsUserConfig::~GamsUserConfig()
{
//    if (mGUCfg)  ymlFree(&mGUCfg);
}

QList<ConfigItem *> GamsUserConfig::readCommandLineParameters()
{
    QList<ConfigItem *> itemList;
//    for (int ipos=0; ipos<ymlGetItemCount(mGUCfg); ipos++) {
//       int gsec, ival, isset;
//       char sval[GMS_SSSIZE], skey[GMS_SSSIZE];

//       if (ymlGetItemKV(mGUCfg, ipos, ymliGSection, &isset, &gsec, sval, sizeof(sval)) > 1)
//           break;

//       if (gsec != CLP)
//           continue;

//       ymlGetItemKV(mGUCfg,ipos,ymlsName,&isset,&ival,skey,sizeof(skey));
//       ymlGetItemKV(mGUCfg,ipos,ymlsValue,&isset,&ival,sval,sizeof(sval));
//       if (isset) {
//           ConfigItem* item = new ConfigItem(QString(skey), QString(sval));
//           ymlGetItemKV(mGUCfg,ipos,ymlsMinVersion,&isset,&ival,skey,sizeof(skey));
//           if (isset)
//              item->minVersion = QString(skey);
//           ymlGetItemKV(mGUCfg,ipos,ymlsMaxVersion,&isset,&ival,skey,sizeof(skey));
//           if (isset)
//               item->maxVersion = QString(skey);
//           itemList.append(item);
//       } else {
//           qDebug() << QString("value for %s is not set").arg(skey);
//       }
//    }
    return itemList;
}

QList<EnvVarConfigItem *> GamsUserConfig::readEnvironmentVariables()
{
    QList<EnvVarConfigItem *> itemList;
//    for (int ipos=0; ipos<ymlGetItemCount(mGUCfg); ipos++) {
//       int gsec, ival, isset;
//       char sval[GMS_SSSIZE], skey[GMS_SSSIZE];

//       if (ymlGetItemKV(mGUCfg, ipos, ymliGSection, &isset, &gsec, sval, sizeof(sval)) > 1)
//           break;

//       if (gsec != EV)
//           continue;

//       ymlGetItemKV(mGUCfg,ipos,ymlsName,&isset,&ival,skey,sizeof(skey));
//       ymlGetItemKV(mGUCfg,ipos,ymlsValue,&isset,&ival,sval,sizeof(sval));
//       if (isset) {
//           EnvVarConfigItem* item = new EnvVarConfigItem(QString(skey), QString(sval));
//           ymlGetItemKV(mGUCfg,ipos,ymlsMinVersion,&isset,&ival,skey,sizeof(skey));
//           if (isset)  item->minVersion = QString(skey);
//           ymlGetItemKV(mGUCfg,ipos,ymlsMaxVersion,&isset,&ival,skey,sizeof(skey));
//           if (isset)  item->maxVersion = QString(skey);
//           ymlGetItemKV(mGUCfg,ipos,ymliPathVariable,&isset,&ival,skey,sizeof(skey));
//           if (isset)
//               item->pathVariable = (ival ? EnvVarConfigItem::pathDefinition::PATH_DEFINED
//                                          : EnvVarConfigItem::pathDefinition::NO_PATH_DEFINED);
//           else
//               item->pathVariable = EnvVarConfigItem::pathDefinition::PATH_UNDEFINED;
//           itemList.append(item);
//       } else {
//           qDebug() << QString("value for %s is not set").arg(skey);
//       }
//    }
    return itemList;
}

} // namespace option
} // namespace studio
} // namespace gams
