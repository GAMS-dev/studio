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
#ifndef CONNECT_H
#define CONNECT_H

#include "connectdata.h"
#include "connecterror.h"
#include "connectschema.h"

namespace gams {
namespace studio {
namespace connect {

class Connect
{
public:
    Connect();
    ~Connect();

    bool validateData(const QString& schemaname, ConnectData& data);

    ConnectData* createDataHolder(const QStringList& schemaNameList);
    void addDataForAgent(ConnectData* data, const QString& schemaName);

    ConnectSchema* getSchema(const QString& schemaName);
    QStringList getSchemaNames() const;

private:
    void listValue(const YAML::Node& schemaValue, YAML::Node& dataValue);
    void mapValue(const YAML::Node& schemaValue, YAML::Node& dataValue);

    YAML::Node createConnectData(const QString& schemaName);

    bool isTypeValid(QList<Type>& typeList, const YAML::Node &data);
    void updateKeyList(const QString& schemaname, QString& keyFromRoot, YAML::Node& error, const YAML::Node &data);

    QMap<QString, ConnectSchema*> mSchema;
    YAML::Node mError;
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // CONNECT_H
