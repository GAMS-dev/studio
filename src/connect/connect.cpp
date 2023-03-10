/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include <QDir>
#include <QMessageBox>
#include <QDebug>

#include "commonpaths.h"
#include "connect.h"
#include "exception.h"

namespace gams {
namespace studio {
namespace connect {

Connect::Connect()
{
    QString connectPath = CommonPaths::gamsConnectSchemaDir();
    if (!QDir(connectPath).exists()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Unable to find Connect Schema Definition");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(QString("%1\n%2").arg("The schema definition files could not be found from gams system directory.",
                                             "The connect editor is unable to function properly." ));
        msgBox.setStandardButtons(QMessageBox::Ok);
        if (msgBox.exec() == QMessageBox::Ok) {
            EXCEPT() << "Unable to find Schema Definition File";
            return;
        }
    }

    const QStringList schemaFiles = QDir(connectPath).entryList(QStringList() << "*.yaml" << "*.yml", QDir::Files);
    for (const QString& filename : schemaFiles) {
        try {
           mSchema[QFileInfo(filename).baseName()] = new ConnectSchema(QDir(connectPath).filePath(filename));
        } catch (std::exception &e) {
            mSchemaError[QDir(connectPath).filePath(filename)] = e.what();
            continue;
        }
    }
    if (mSchemaError.size()>0) {
        QStringList keys = mSchemaError.keys();
        QMessageBox msgBox;
        msgBox.setWindowTitle("Unable to read Schema Definition File");
        msgBox.setText("Warning");
        msgBox.setIcon(QMessageBox::Warning);
        if (mSchemaError.size()==1) {
            msgBox.setText("Schema \""+ QFileInfo(keys.first()).baseName() + "\" read from \""
                                      + QDir(connectPath).filePath(keys.first()) + "\" contains an unsupported/invalid rule. \n"
                           + "Data using a schema from this file may not display correctly.\n"+
                           + "You can reopen the file using text editor to edit the content."    );
        } else {
//            QString msg("An unsupported/invalid schema read from [" + keys.join(",")
//                                                                    + "] contains an unsupported/invalid rule. \n"
//                           + "Data using a schema from this file may not display correctly.\n"+
//                           + "You can reopen the file using text editor to edit the content."    );
        }
        msgBox.setStandardButtons(QMessageBox::Ok);
        if (msgBox.exec() == QMessageBox::Ok)
            return;
    }
}

Connect::~Connect()
{
    mSchemaError.clear();
    if (mSchema.size()>0) {
       qDeleteAll( mSchema );
       mSchema.clear();
    }
}

bool Connect::validateData(const QString &inputFileName, bool checkSchema)
{
    YAML::Node error = YAML::Node(YAML::NodeType::Map);
    try {
        YAML::Node node = YAML::LoadFile(inputFileName.toStdString());
        if (node.Type()!=YAML::NodeType::Sequence)
            return false;

        bool invalidSchema = true;
        for (size_t i=0; i<node.size(); i++) {
            if (node[i].Type()!=YAML::NodeType::Map) {
                invalidSchema = true;
                break;
            } else {
                for (YAML::const_iterator it = node[i].begin(); it != node[i].end(); ++it) {
                    QString key;
                    try {
                       std::string str = it->first.as<std::string>();
                       key = QString::fromStdString(str);
                    } catch(const YAML::BadConversion& e) {
                        Q_UNUSED(e);
                        invalidSchema = true;
                        break;
                    }
                    if (mSchema.contains( key )) {
                        if (checkSchema) {
                            ConnectData data = node[i];
                            invalidSchema = (validate( key, data )? false: true);
                        } else{
                           invalidSchema = false;
                        }
                    } else {
                        YAML::Node errorNode = YAML::Node(YAML::NodeType::Sequence);
                        errorNode.push_back( QString("unknonwn schema" ).toStdString() );
                        error[key.toStdString()] = errorNode;

                        invalidSchema = true;
                    }
                }
            }
        }
        mError =  ConnectError(error);
        return (!invalidSchema);

    } catch(const YAML::ParserException& e) {
        Q_UNUSED(e);
        return false;
    }
}

ConnectData *Connect::loadDataFromFile(const QString &fileName)
{
    return new ConnectData(fileName);
}

bool Connect::validate(const QString &schemaname, ConnectData &data)
{
    YAML::Node datanode = data.getRootNode();
    if (datanode.Type()!=YAML::NodeType::Map) {
        return false;
    }

    QStringList schemaKeylist = getSchema(schemaname)->getFirstLevelKeyList();
    YAML::Node error;
    for (YAML::const_iterator it = data.getRootNode().begin(); it != data.getRootNode().end(); ++it) {
        QString key = QString::fromStdString( it->first.as<std::string>() );
        if (!schemaKeylist.contains(key)) {
            if(getSchema(schemaname)->isRequired(key)) {
               YAML::Node errorNode = YAML::Node(YAML::NodeType::Sequence);
               errorNode.push_back( QString("required field" ).toStdString() );
               error[key.toStdString()] = errorNode;
            }

            bool validType = false;
            const QList<SchemaType> typeList = getSchema(schemaname)->getType(key);
            for (SchemaType type : typeList) {
                try {
                    if (type==SchemaType::Integer) {
                        if (it->second.Type()==YAML::NodeType::Scalar)
                            it->second.as<int>();
                        else
                            continue;
                    } else if (type==SchemaType::Float) {
                              if (it->second.Type()==YAML::NodeType::Scalar)
                                 it->second.as<float>();
                              else
                                 continue;
                    } else if (type==SchemaType::Boolean) {
                               if (it->second.Type()==YAML::NodeType::Scalar)
                                   it->second.as<bool>();
                               else
                                   continue;
                    } else if (type==SchemaType::String) {
                              if (it->second.Type()==YAML::NodeType::Scalar)
                                  it->second.as<std::string>();
                              else
                                  continue;
                    } else if (type==SchemaType::List) {
                              continue; // TODO
                    } else if (type==SchemaType::Dict) {
                              continue; // TODO
                    }
                    validType = true;
                    break;
                } catch (const YAML::BadConversion &) {
                    validType=false;
                }
           }
           if (!validType) {
               std::string str = "must be of ";
               if (typeList.size()==1) {
                   str += ConnectSchema::typeToString(typeList[0]);
                   str += " type";
               } else {
                   str += "[";
                   int i = 0;
                   for (auto const& t : typeList) {
                       ++i;
                       str += ConnectSchema::typeToString(t);
                       if (i < typeList.size())
                          str += ",";
                   }
                   str += "] type";
               }
               YAML::Node errorNode = YAML::Node(YAML::NodeType::Sequence);
               errorNode.push_back( QString("required field" ).toStdString() );
               error[key.toStdString()] = errorNode;
           }
        }
    }
    mError =  ConnectError(error);
    return (error.size()== 0);
}

ConnectData *Connect::createDataHolder(const QStringList &schemaNameList, bool onlyRequiredAttribute)
{
    int i = 0;
    YAML::Node data = YAML::Node(YAML::NodeType::Sequence);
    for (const QString &name : schemaNameList) {
        YAML::Node node = YAML::Node(YAML::NodeType::Map);
        node[name.toStdString()] =  createConnectData(name, onlyRequiredAttribute);
        data[i++] = node;
    }
    return new ConnectData(data);
}

ConnectData *Connect::createDataHolderFromSchema(const QString& schemaname, const QStringList &schema, bool onlyRequiredAttribute)
{
    ConnectSchema* s = mSchema[schemaname];
    YAML::Node data;
    if (!s)
        return new ConnectData(data);

    QString schemastr = schema.join(":");

    Schema* schemaHelper = s->getSchema(schemastr);
    if (!schemaHelper)
        return new ConnectData(data);

    YAML::Node schemanode = schemaHelper->schemaNode;
    YAML::Node value;
    if (mapValue( schemanode, value, true, onlyRequiredAttribute )) {
        data = value;
    }
    return new ConnectData(data);
}

ConnectData *Connect::createDataHolderFromSchema(const QStringList &schemastrlist, bool onlyRequiredAttribute)
{
    QString schemaname = schemastrlist.first();
    ConnectSchema* s = mSchema[schemaname];
    YAML::Node data;
    if (!s)
        return new ConnectData(data);

    QStringList tobeinsertSchemaKey(schemastrlist);
    tobeinsertSchemaKey.removeFirst();
    QString schemastr = tobeinsertSchemaKey.join(":");
    Schema* schemaHelper = s->getSchema(schemastr);
    if (!schemaHelper)
        return new ConnectData(data);

    YAML::Node schemanode = schemaHelper->schemaNode;
    YAML::Emitter e;
    e << schemanode;
    YAML::Node value;
    if (mapValue( schemanode, value, true, onlyRequiredAttribute )) {
        data[tobeinsertSchemaKey.last().toStdString()] = value;
    }
    return new ConnectData(data);
}

ConnectSchema *Connect::getSchema(const QString &schemaName)
{
    if (mSchema.contains(schemaName))
        return mSchema[schemaName];
    else
        return nullptr;
}

const QStringList Connect::getSchemaNames() const
{
    return mSchema.keys();
}

bool Connect::isSchemaAvaiablel() const
{
    return !mSchema.isEmpty();
}

ConnectError Connect::getError() const
{
    return mError;
}

bool Connect::listValue(const YAML::Node &schemaValue, YAML::Node &dataValue, bool ignoreRequiredSchema, bool onlyRequiredAttribute)
{
    bool allowed = (ignoreRequiredSchema ? ignoreRequiredSchema
                                         : schemaValue["required"] ? (onlyRequiredAttribute ? schemaValue["required"].as<bool>() : true)
                                                                   : (onlyRequiredAttribute ? false : true)
                   );
    if (schemaValue["type"]) {
        if (schemaValue["type"].Type()==YAML::NodeType::Sequence) {
            std::string str = schemaValue["type"][0].as<std::string>();
            if (allowed && str.compare("integer") == 0) {
                dataValue[0] = 0;
            } else if (allowed && str.compare("boolean") == 0) {
                     dataValue[0] = false;
            } else {
                if (allowed)
                    dataValue[0] = "[value]";
                else
                    return false;
            }
        }  else {
            std::string value = schemaValue["type"].as<std::string>() ;
            if (allowed && value.compare("dict") == 0) {
                if (schemaValue["schema"]) {
                    YAML::Node node;
                    for (YAML::const_iterator it = schemaValue["schema"].begin(); it != schemaValue["schema"].end(); ++it) {
                        if (it->second.Type() == YAML::NodeType::Map) {
                            //Key key;
                            YAML::Node value;
                            if (!mapValue( it->second, value, false, onlyRequiredAttribute ))
                                continue;
                            try {
                                int i = it->first.as<int>();
                                node[i] = value;
                            } catch (const YAML::BadConversion& e) {
                                Q_UNUSED(e);
                                std::string s = it->first.as<std::string>();
                                node[s] = value;
                            }
                        }
                        // else TODO
                    }
                    dataValue[0] = node;
                } else if (schemaValue["oneof_schema"]) {
                    YAML::Node oneofnode = schemaValue["oneof_schema"][0];
                    if (oneofnode.Type() == YAML::NodeType::Map) {
                        YAML::Node data;
                        if (!mapValue( oneofnode, data, false, onlyRequiredAttribute ))
                            return false;
                        dataValue[0] = data;
                    }
                }
            } else if (allowed && value.compare("string") == 0) {
                       dataValue[0] = "[value]";
            } else if (allowed && value.compare("integer") == 0) {
                       dataValue[0] = 0;
            } else {
                if (allowed)
                    dataValue[0] = "[value]";
                else
                    return false;
            }
        }
    }
    return true;
}

bool Connect::mapValue(const YAML::Node &schemaValue, YAML::Node &dataValue, bool ignoreRequiredSchema, bool onlyRequiredAttribute)
{
    if (schemaValue.Type() == YAML::NodeType::Map) {
        bool allowed = (ignoreRequiredSchema ? ignoreRequiredSchema
                                             : schemaValue["required"] ? (onlyRequiredAttribute ? schemaValue["required"].as<bool>() : true)
                                                                       : (onlyRequiredAttribute ? false : true)
                       );
        if (schemaValue["type"]) {
            if (schemaValue["type"].Type()==YAML::NodeType::Sequence) {
/*                if (schemaValue["schema"]) {
                    qDebug() << "schema sequence";
                    YAML::Node data;
                    if (listValue(schemaValue["schema"], dataValue, true, onlyRequiredAttribute))
                        dataValue = data;
                    else
                        return false;
                } else { */
                    YAML::Node firsttype = schemaValue["type"][0];
                    std::string value = firsttype.as<std::string>() ;
                    if (allowed &&value.compare("list") == 0) {
                        dataValue[0] = "[value]";
                    } else if (allowed &&value.compare("boolean") == 0) {
                           dataValue = (schemaValue["default"] ? schemaValue["default"].as<bool>() : false);
                    } else if (allowed &&value.compare("integer") == 0) {
                        dataValue = (schemaValue["default"] ? schemaValue["default"].as<int>() :  0);;
                    } else {
                        if (allowed)
                            dataValue = "[value]";
                        else
                            if (!listValue(schemaValue["schema"], dataValue, true, onlyRequiredAttribute))
                                return false;
                            return false;
                    }
//                }
            } else { // not sequence
                std::string value = schemaValue["type"].as<std::string>() ;
                if (allowed && value.compare("string") == 0) {
                    if (schemaValue["default"]) {
                        dataValue = schemaValue["default"].as<std::string>();
                    } else  if (schemaValue["allowed"] && schemaValue["allowed"].Type()==YAML::NodeType::Sequence) {
                        std::string str = schemaValue["allowed"][0].as<std::string>();
                        dataValue = str;
                        for(size_t i=0; i<schemaValue["allowed"].size(); i++) {
                            std::string str = schemaValue["allowed"][i].as<std::string>();
                            if (str.compare(std::string("default"))==0) {
                               dataValue = "default";
                               break;
                            }
                        }
                    } else {
                        dataValue = "[value]";
                    }
                } else if (allowed && value.compare("integer") == 0) {
                          dataValue = (schemaValue["default"] ? schemaValue["default"].as<int>() :  0);
                } else if (allowed && value.compare("boolean") == 0) {
                          dataValue = (schemaValue["default"] ? schemaValue["default"].as<bool>() : false);
                } else if (allowed && value.compare("dict") == 0) {
                           if (schemaValue["schema"]) {
                               YAML::Node data;
                               if (mapValue(schemaValue["schema"], data, false, onlyRequiredAttribute))
                                  dataValue = data;
                               else
                                   return false;
                           } else {
                               dataValue["[key]"] = "[value]";
                           }
                } else if (allowed && value.compare("list") == 0) {
                           if (schemaValue["schema"]) {
                               if (!listValue(schemaValue["schema"], dataValue, true, onlyRequiredAttribute))
                                   return false;
                           } else {
                               if (allowed)
                                   dataValue[0] = 0;
                           }
                } else {
                    if (allowed)
                        dataValue = "[value]";
                    else
                        return false;
                }
            }
        } else { // not schema["type"]
            if (schemaValue["anyof"]) {
                if (schemaValue["anyof"].Type()==YAML::NodeType::Sequence) {
                    YAML::Node anyofnode = schemaValue["anyof"][0];
                    if (anyofnode.Type() == YAML::NodeType::Map) {
                        if (!mapValue( anyofnode, dataValue, false, onlyRequiredAttribute ))
                            return false;
                    }
                }
            } else {
                int i=0;
                for (YAML::const_iterator it = schemaValue.begin(); it != schemaValue.end(); ++it) {
                    if (it->second.Type() == YAML::NodeType::Map) {
                        YAML::Node value;
                        if (!mapValue( it->second, value, false, onlyRequiredAttribute ))
                            continue;
                        try {
                            int i = it->first.as<int>();
                            dataValue[i] = value;
                        } catch (const YAML::BadConversion& e) {
                            Q_UNUSED(e);
                            std::string s = it->first.as<std::string>();
                            dataValue[s] = value;
                        }
                    }
                }
            }
        }
    }
    return true;
}

YAML::Node Connect::getDefaultValueByType(Schema* schemaHelper)
{
    SchemaType type = schemaHelper->types.at(0);
    QList<ValueWrapper> allowedValues = schemaHelper->allowedValues;
    ValueWrapper defaultValue = schemaHelper->defaultValue;
    YAML::Node node = YAML::Node(YAML::NodeType::Scalar);
    switch (type) {
    case SchemaType::String: {
        std::string str = "[value]";
        for(int i=0; i<allowedValues.size(); i++) {
            ValueWrapper str = allowedValues.at(0);
            if (std::string(str.value.stringval).compare("default")==0) {
               str = "default";
               break;
            }
        }
        node = ( defaultValue.type==SchemaValueType::NoValue
                              ? (allowedValues.size() > 0 ? std::string(allowedValues.at(0).value.stringval) : str)
                              : std::string(defaultValue.value.stringval) );
        break;
    }
    case SchemaType::Integer: {
        node = ( defaultValue.type==SchemaValueType::NoValue
                              ? (allowedValues.size() > 0 ? allowedValues.at(0).value.intval : 0)
                              : defaultValue.value.intval );
        break;
    }
    case SchemaType::Float: {
        node = ( defaultValue.type==SchemaValueType::NoValue
                              ? (allowedValues.size() > 0 ? allowedValues.at(0).value.doubleval : 0.0)
                              : defaultValue.value.doubleval );
        break;
    }
    case SchemaType::Boolean: {
        node = (defaultValue.type==SchemaValueType::NoValue
                              ? (allowedValues.size() > 0 ? allowedValues.at(0).value.boolval : false)
                              : defaultValue.value.boolval );
        break;
    }
    default: { // string
        node = "[value]";
        break;
    }
    }
    return node;
}

YAML::Node Connect::createConnectData(const QString &schemaName, bool onlyRequiredAttribute)
{
    YAML::Node data = YAML::Node(YAML::NodeType::Map);
    ConnectSchema* s = mSchema[schemaName];
    for (YAML::const_iterator it = s->mRootNode.begin(); it != s->mRootNode.end(); ++it) {
        if (it->second.Type() == YAML::NodeType::Map) { // first level should be a map
            YAML::Node value;
            if (!mapValue( it->second, value, false, onlyRequiredAttribute ))
                continue;
            try {
                int i = it->first.as<int>();
                data[i] = value;
            } catch (const YAML::BadConversion& e) {
                Q_UNUSED(e);
                std::string s = it->first.as<std::string>();
                data[s] = value;
            }
        }
    }
//    YAML::Node connectdata = YAML::Node(YAML::NodeType::Sequence);
//    connectdata[0] = data;
    return data;
}

bool Connect::isTypeValid(const QList<SchemaType>& typeList, const YAML::Node &data)
{
    bool validType = false;
    for (SchemaType t : typeList) {
        try {
            if (t==SchemaType::Integer) {
                if (data.Type()!=YAML::NodeType::Scalar)
                    continue;
                data.as<int>();
            } else if (t==SchemaType::Float) {
                      if (data.Type()!=YAML::NodeType::Scalar)
                          continue;
                      data.as<float>();
            } else if (t==SchemaType::Boolean) {
                      if (data.Type()!=YAML::NodeType::Scalar)
                          continue;
                       data.as<bool>();
            } else if (t==SchemaType::String) {
                      if (data.Type()!=YAML::NodeType::Scalar)
                          continue;
                      data.as<std::string>();
            } else if (t==SchemaType::List) {
                      if (data.Type()!=YAML::NodeType::Sequence)
                          continue;
            } else if (t==SchemaType::Dict) {
                      if (data.Type()!=YAML::NodeType::Map)
                         continue;
            }
            validType = true;
            break;
        } catch (const YAML::BadConversion& e) {
            Q_UNUSED(e);
            validType=false;
        }
   }
    return validType;
}

} // namespace connect
} // namespace studio
} // namespace gams
