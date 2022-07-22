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
#include "connectdata.h"
#include "connectschema.h"
#include "connecterror.h"

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

bool Connect::validateData(const QString &schemaname, ConnectData &data)
{
    YAML::Node datanode = data.getRootNode();
    if (datanode.Type()!=YAML::NodeType::Map) {
        return false;
    }

    QStringList schemaKeylist = getSchema(schemaname)->getFirstLevelKeyList();
//    qDebug() << "firstlevelkeylist::" << schemaKeylist;
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
            QList<Type> typeList = getSchema(schemaname)->getType(key);
            foreach (Type type, typeList) {
                try {
                    if (type==Type::INTEGER) {
                        if (it->second.Type()==YAML::NodeType::Scalar)
                            it->second.as<int>();
                        else
                            continue;
                    } else if (type==Type::FLOAT) {
                              if (it->second.Type()==YAML::NodeType::Scalar)
                                 it->second.as<float>();
                              else
                                 continue;
                    } else if (type==Type::BOOLEAN) {
                               if (it->second.Type()==YAML::NodeType::Scalar)
                                   it->second.as<bool>();
                               else
                                   continue;
                    } else if (type==Type::STRING) {
                              if (it->second.Type()==YAML::NodeType::Scalar)
                                  it->second.as<std::string>();
                              else
                                  continue;
                    } else if (type==Type::LIST) {
                              continue; // TODO
                    } else if (type==Type::DICT) {
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
                   str += typeToString(typeList[0]);
                   str += " type";
               } else {
                   str += "[";
                   int i = 0;
                   for(auto const& t: typeList) {
                       ++i;
                       str += typeToString(t);
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
                dataValue[0] = "";
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
                       dataValue[0] = "";
            } else if (value.compare("integer") == 0) {
                       dataValue[0] = 0;
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
                }
            } else {
                std::string value = schemaValue["type"].as<std::string>() ;
                if (value.compare("string") == 0) {
                    // allowed
                    if (schemaValue["allowed"] && schemaValue["allowed"].Type()==YAML::NodeType::Sequence) {
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
                        dataValue = "";
                    }
                } else if (value.compare("integer") == 0) {
                        dataValue = 0;
                } else if (value.compare("boolean") == 0) {
                        dataValue = false;
                } else if (value.compare("dict") == 0) {
                           if (schemaValue["schema"]) {
                               mapValue(schemaValue["schema"], dataValue);
                           } else {
                               dataValue[""] = "";
                           }
                } else if (value.compare("list") == 0) {
                           if (schemaValue["schema"]) {
                               listValue(schemaValue["schema"], dataValue);
                           }
                }
            }
        }
    }
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
    YAML::Node connectdata = YAML::Node(YAML::NodeType::Sequence);
    connectdata[0] = data;
    return connectdata;
}

bool Connect::isTypeValid(QList<Type>& typeList, const YAML::Node &data)
{
    bool validType = false;
    foreach (Type t, typeList) {
        try {
            if (t==Type::INTEGER) {
                if (data.Type()!=YAML::NodeType::Scalar)
                    continue;
                data.as<int>();
            } else if (t==Type::FLOAT) {
                      if (data.Type()!=YAML::NodeType::Scalar)
                          continue;
                      data.as<float>();
            } else if (t==Type::BOOLEAN) {
                      if (data.Type()!=YAML::NodeType::Scalar)
                          continue;
                       data.as<bool>();
            } else if (t==Type::STRING) {
                      if (data.Type()!=YAML::NodeType::Scalar)
                          continue;
                      data.as<std::string>();
            } else if (t==Type::LIST) {
                      if (data.Type()!=YAML::NodeType::Sequence)
                          continue;
            } else if (t==Type::DICT) {
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
            QList<Type> typeList = getSchema(schemaname)->getType(keyFromRoot);
            bool validType = isTypeValid(typeList, it->second);
            if (!validType) {
                QString str = "must be of ";
                if (typeList.size()==1) {
                    str += typeToString(typeList[0]);
                    str += " type";
                } else {
                    str += "[";
                    int i = 0;
                    for(auto const& t: typeList) {
                        ++i;
                        str += typeToString(t);
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
                if (minval.type==ValueType::INTEGER) {
                    if (data.as<int>() < minval.value.intval) {
                        /* TODO */
                    }
                } else if (minval.type==ValueType::FLOAT) {
                        if (data.as<float>() < minval.value.doubleval) {
                            /* TODO */
                        }
                }
                ValueWrapper maxval = getSchema(schemaname)->getMax(keyFromRoot);
                if (maxval.type!=ValueType::NOVALUE) {

                }
            }
        } else if (it->second.Type()==YAML::NodeType::Sequence) {
                   QString key = QString("%1:-").arg(keyFromRoot);
                   QList<Type> typeList = getSchema(schemaname)->getType(key);
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
