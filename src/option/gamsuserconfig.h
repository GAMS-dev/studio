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
 */
#ifndef GAMSUSERCONFIG_H
#define GAMSUSERCONFIG_H

#include <QObject>

#include "gclgms.h"
#include "guccc.h"
#include "gucapi.h"

namespace gams {
namespace studio {
namespace option {

class ConfigItem {

public:
    ConfigItem() {}
    ConfigItem(QString k, QString v) : key(k), value(v) {}
    ConfigItem(QString k, QString v, QString min, QString max) :
        key(k), value(v), minVersion(min), maxVersion(max) {}

    QString key = "";
    QString value = "";
    QString minVersion = "";
    QString maxVersion = "";
};

class EnvVarConfigItem : public ConfigItem {
public:
    enum pathDefinition {
        NONE = -1,
        PATH_DEFINED = 0,
        NO_PATH_DEFINED =1
    };
    pathDefinition pathVariable = NONE;

    EnvVarConfigItem() { }
    EnvVarConfigItem(QString k, QString v): ConfigItem(k, v)  { }
    EnvVarConfigItem(QString k, QString v, pathDefinition path): ConfigItem(k, v), pathVariable(path) { }
    bool hasPathVariable() { return (pathVariable != NONE); }
};

class GamsUserConfig : public QObject
{
    Q_OBJECT
public:
    GamsUserConfig(const QString &location);
    ~GamsUserConfig();

    QList<EnvVarConfigItem *> readEnvironmentVariables();
    QList<ConfigItem *> readCommandLineParameters();

    void updateCommandLineParameters(const QList<ConfigItem *> &items);
    void updateEnvironmentVariables(const QList<EnvVarConfigItem *> &items);

    void writeGamsUserConfigFile(const QString &location);

    bool reloadGAMSUserConfigFile(const QString &location);

    bool isAvailable() const;
    QString getLastErrorMessage() const;

private:
    bool readGAMSUserConfigFile(const QString &location);
    void clearLastErrorMessage();
    void setLastErrorMessage(const char *message);
    void setLastErrorMessage(const QString &message);
    static int errorCallback(int count, const char *message);

    QString mLastErrorMessage;
    QString mLocation;
    bool mAvailable;
    gucHandle_t mGUCfg;

};

} // namespace option
} // namespace studio
} // namespace gams
#endif // GAMSUSERCONFIG_H
