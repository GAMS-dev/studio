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

    bool validateData(const QString& inputFileName, bool checkSchema=false);

    ConnectData* loadDataFromFile(const QString& fileName);

    ConnectData* createDataHolder(const QStringList& schemaNameList, bool onlyRequiredAttribute=false);
    ConnectData* createDataHolderFromSchema(const QString& schemaname, const QStringList& schema, bool onlyRequiredAttribute=false, bool ignoreNull=false);
    ConnectData* createDataHolderFromSchema(const QStringList& schemastrlist, bool onlyRequiredAttribute=false, bool ignoreNull=false);

    ConnectSchema* getSchema(const QString& schemaName);
    const QStringList getSchemaNames() const;

    bool isSchemaAvaiablel() const;

    ConnectError getError() const;

private:
    bool listValue(const YAML::Node& schemaValue, YAML::Node& dataValue, bool ignoreRequiredSchema=false, bool onlyRequiredAttribute=false);
    bool mapValue(const YAML::Node& schemaValue, YAML::Node& dataValue, bool ignoreRequiredSchema=false, bool onlyRequiredAttribute=false, bool ignoreNull=false);

    bool mapTypeSequenceValue(const YAML::Node& typenode, const YAML::Node& schemaValue, YAML::Node& dataValue,
                              bool onlyRequiredAttribute=false, bool allowed=false);

    YAML::Node getDefaultValueByType(Schema* schemaHelper);

    YAML::Node createConnectData(const QString& schemaName, bool onlyRequiredAttribute=false);
    bool validate(const QString& schemaname, ConnectData& data);

    bool isTypeValid(const QList<SchemaType> &typeList, const YAML::Node &data);

    QMap<QString, ConnectSchema*> mSchema;
    QMap<QString, QString> mSchemaError;
    ConnectError mError;
    static QRegularExpression mRex;
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // CONNECT_H
