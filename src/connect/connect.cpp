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
#include <QDir>
#include <QDebug>

#include "commonpaths.h"
#include "connect.h"

namespace gams {
namespace studio {
namespace connect {

Connect::Connect()
{
    QString connectPath = QDir::cleanPath(CommonPaths::systemDir()+QDir::separator()+ CommonPaths::gamsConnectSchemaDir());
    QStringList schemaFiles = QDir(connectPath).entryList(QStringList() << "*.yaml" << "*.yml", QDir::Files);
    foreach(const QString& filename, schemaFiles) {
       mSchema[QFileInfo(filename).baseName()] = new ConnectSchema(QDir(connectPath).filePath(filename));
    }
}

Connect::~Connect()
{
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
                        invalidSchema = true;
                        break;
                    }
                    if (mSchema.keys().contains( key )) {
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
            QList<SchemaType> typeList = getSchema(schemaname)->getType(key);
            foreach (SchemaType type, typeList) {
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
                } catch (const YAML::BadConversion& e) {
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
                   foreach(auto const& t, typeList) {
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

ConnectData *Connect::createDataHolder(const QStringList &schemaNameList)
{
    int i = 0;
    YAML::Node data = YAML::Node(YAML::NodeType::Sequence);
    foreach(QString name, schemaNameList) {
        YAML::Node node = YAML::Node(YAML::NodeType::Map);
        node[name.toStdString()] =  createConnectData(name);
        data[i++] = node;
    }
    return new ConnectData(data);
}

ConnectData *Connect::createDataHolderFromSchema(const QString& schemaname, const QStringList &schema)
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
    YAML::Emitter e;
    e << schemanode;
    mapValue( schemanode, data );
    return new ConnectData(data);
}

ConnectData *Connect::createDataHolderFromSchema(const QStringList &schemastrlist)
{
    qDebug() << "  __x_ 0 schemstrlist=" << schemastrlist;
    QString schemaname = schemastrlist.first();
    ConnectSchema* s = mSchema[schemaname];
    YAML::Node data;
    if (!s)
        return new ConnectData(data);

    QStringList tobeinsertSchemaKey(schemastrlist);
    tobeinsertSchemaKey.removeFirst();
    QString schemastr = tobeinsertSchemaKey.join(":");
    qDebug() << "  __x_ 1 schemastr=" << schemastr;
    Schema* schemaHelper = s->getSchema(schemastr);
    if (!schemaHelper)
        return new ConnectData(data);

    qDebug() << "  __x_ 2 schemaHelper=" << schemaHelper->level;
    YAML::Node schemanode = schemaHelper->schemaNode;
    YAML::Emitter e;
    e << schemanode;
    qDebug() << "  __x_ 3 schemanode=" << e.c_str();
    YAML::Node value;
    mapValue( schemanode, value );
    data[tobeinsertSchemaKey.last().toStdString()] = value;
    return new ConnectData(data);
}

void Connect::addDataForAgent(ConnectData *data, const QString &schemaName)
{
    Q_ASSERT(data->getRootNode().Type()==YAML::NodeType::Sequence);
    int i = data->getRootNode().size();
    YAML::Node node = YAML::Node(YAML::NodeType::Map);
    node[schemaName.toStdString()] =  createConnectData(schemaName);
    data[i] = node;
}

ConnectSchema *Connect::getSchema(const QString &schemaName)
{
    if (mSchema.contains(schemaName))
        return mSchema[schemaName];
    else
        return nullptr;
}

QStringList Connect::getSchemaNames() const
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

void Connect::listValue(const YAML::Node &schemaValue, YAML::Node &dataValue)
{
    if (schemaValue["type"]) {
        if (schemaValue["type"].Type()==YAML::NodeType::Sequence) {
            std::string str = schemaValue["type"][0].as<std::string>();
            if (str.compare("integer") == 0) {
                dataValue[0] = 0;
            } else if (str.compare("boolean") == 0) {
                     dataValue[0] = false;
            } else {
                dataValue[0] = "[value]";
            }
        }  else {
            std::string value = schemaValue["type"].as<std::string>() ;
            if (value.compare("dict") == 0) {
                if (schemaValue["schema"]) {
                    YAML::Node node;
                    for (YAML::const_iterator it = schemaValue["schema"].begin(); it != schemaValue["schema"].end(); ++it) {
                        if (it->second.Type() == YAML::NodeType::Map) {
                            //Key key;
                            YAML::Node value;
                            mapValue( it->second, value );
                            try {
                                int i = it->first.as<int>();
                                node[i] = value;
                            } catch (const YAML::BadConversion& e) {
                                std::string s = it->first.as<std::string>();
                                node[s] = value;
                            }
                        }
                        // else TODO
                    }
                    dataValue[0] = node;
                }
            } else if (value.compare("string") == 0) {
                       dataValue[0] = "[value]";
            } else if (value.compare("integer") == 0) {
                       dataValue[0] = 0;
            } else {
                dataValue[0] = "[value]";
            }
        }
    }
}

void Connect::mapValue(const YAML::Node &schemaValue, YAML::Node &dataValue)
{
    if (schemaValue.Type() == YAML::NodeType::Map) {
        if (schemaValue["type"]) {
            if (schemaValue["type"].Type()==YAML::NodeType::Sequence) {
                if (schemaValue["schema"]) {
                    listValue(schemaValue["schema"], dataValue);
                } else {
                    YAML::Node firsttype = schemaValue["type"][0];
                    std::string value = firsttype.as<std::string>() ;
                    if (value.compare("list") == 0) {
                        dataValue[0] = "[value]";
                    } else if (value.compare("boolean") == 0) {
                           dataValue["[key]"] = "[value]";
                    } else if (value.compare("integer") == 0) {
                        dataValue = 0;
                    } else if (value.compare("boolean") == 0) {
                         dataValue = false;
                    } else {
                         dataValue = "[value]";
                    }
                }
            } else {
                std::string value = schemaValue["type"].as<std::string>() ;
                if (value.compare("string") == 0) {
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
                } else if (value.compare("integer") == 0) {
                          dataValue = (schemaValue["default"] ? schemaValue["default"].as<int>() :  0);
                } else if (value.compare("boolean") == 0) {
                          dataValue = (schemaValue["default"] ? schemaValue["default"].as<bool>() : false);
                } else if (value.compare("dict") == 0) {
                           if (schemaValue["schema"]) {
                               mapValue(schemaValue["schema"], dataValue);
                           } else {
                               dataValue["[key]"] = "[value]";
                           }
                } else if (value.compare("list") == 0) {
                           if (schemaValue["schema"]) {
                               listValue(schemaValue["schema"], dataValue);
                           }
                } else {
                    dataValue = "[value]";
                }
            }
        }
    }
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

YAML::Node Connect::createConnectData(const QString &schemaName)
{
    YAML::Node data = YAML::Node(YAML::NodeType::Map);
    ConnectSchema* s = mSchema[schemaName];
    for (YAML::const_iterator it = s->mRootNode.begin(); it != s->mRootNode.end(); ++it) {
        if (it->second.Type() == YAML::NodeType::Map) { // first level should be a map
            YAML::Node value;
            mapValue( it->second, value );
            try {
                int i = it->first.as<int>();
                data[i] = value;
            } catch (const YAML::BadConversion& e) {
                std::string s = it->first.as<std::string>();
                data[s] = value;
            }
        }
    }
//    YAML::Node connectdata = YAML::Node(YAML::NodeType::Sequence);
//    connectdata[0] = data;
    return data;
}

bool Connect::isTypeValid(QList<SchemaType>& typeList, const YAML::Node &data)
{
    bool validType = false;
    foreach (SchemaType t, typeList) {
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
            validType=false;
        }
   }
    return validType;
}

void Connect::updateKeyList(const QString& schemaname, QString& keyFromRoot, YAML::Node& error, const YAML::Node &data)
{
    for (YAML::const_iterator it = data.begin(); it != data.end(); ++it) {
        QString key = QString::fromStdString( it->first.as<std::string>() );
        if (it->second.Type()==YAML::NodeType::Scalar) {
            QList<SchemaType> typeList = getSchema(schemaname)->getType(keyFromRoot);
            bool validType = isTypeValid(typeList, it->second);
            if (!validType) {
                QString str = "must be of ";
                if (typeList.size()==1) {
                    str += ConnectSchema::typeToString(typeList[0]);
                    str += " type";
                } else {
                    str += "[";
                    int i = 0;
                    for(auto const& t: typeList) {
                        ++i;
                        str += ConnectSchema::typeToString(t);
                        if (i < typeList.size())
                           str += ",";
                    }
                    str += "] type";
                }
                YAML::Node errorNode = YAML::Node(YAML::NodeType::Sequence);
                errorNode.push_back( QString("must be of type %1" ).arg(str).toStdString() );
                error[key.toStdString()] = errorNode;
            } else {
                ValueWrapper minval = getSchema(schemaname)->getMin(keyFromRoot);
                try {
                    if (minval.type==SchemaValueType::Integer) {
                        if (data.as<int>() < minval.value.intval) {
                            /* TODO */
                        }
                    } else if (minval.type==SchemaValueType::Float) {
                            if (data.as<float>() < minval.value.doubleval) {
                                /* TODO */
                            }
                    }
                } catch(const YAML::BadConversion& e) {
                    qDebug() << "bad conversion :: minval";
                }
                ValueWrapper maxval = getSchema(schemaname)->getMax(keyFromRoot);
                if (maxval.type!=SchemaValueType::NoValue) {

                }
            }
        } else if (it->second.Type()==YAML::NodeType::Sequence) {
                   QString key = QString("%1:-").arg(keyFromRoot);
                   QList<SchemaType> typeList = getSchema(schemaname)->getType(key);
                   YAML::Node listError;
                   for(size_t i = 0; i<it->second.size(); i++) {
                       YAML::Node itemError;
                       updateKeyList(schemaname, key, itemError, it->second[i]);
                       if (itemError.size() > 0) {
                           YAML::Node nodeError = YAML::Node(YAML::NodeType::Map);
                           nodeError[i] = itemError;
                       }
                   }
                   if (listError.size() > 0) {
                       YAML::Node errorNode = YAML::Node(YAML::NodeType::Sequence);
                       errorNode.push_back( listError );
                       error[keyFromRoot.toStdString()] = errorNode;
                  }
        } else if (it->second.Type()==YAML::NodeType::Map) {
                  keyFromRoot = QString("%1:").arg(keyFromRoot);
                  updateKeyList(schemaname, keyFromRoot, error, it->second );
        }
    }
}

} // namespace connect
} // namespace studio
} // namespace gams
