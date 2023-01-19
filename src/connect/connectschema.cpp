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
#include <QDebug>
#include "connectschema.h"
#include "exception.h"

namespace gams {
namespace studio {
namespace connect {


void ConnectSchema::createSchemaHelper(QString& key, const YAML::Node& node, int level)
{
    if (!mOrderedKeyList.contains(key))
        mOrderedKeyList << key;

    QList<SchemaType> types;
    if (node["type"]) {
        if (node["type"].Type()==YAML::NodeType::Sequence) {
            for(size_t i=0; i<node["type"].size(); i++) {
               QString value(QString::fromStdString(node["type"][i].as<std::string>()));
               if (node["type"][i])
                   types << getTypeFromValue(value);
            }
        } else if (node["type"].Type()==YAML::NodeType::Scalar) {
                  QString value(QString::fromStdString(node["type"].as<std::string>()));
                  types << getTypeFromValue(value);
        }
    }
    bool required = false;
    if (node["required"]) {
        required = (node["required"].as<std::string>().compare("true") == 0 || node["required"].as<std::string>().compare("True") == 0);
    }
    QList<ValueWrapper> allowedValues;
    if (node["allowed"] && node.Type() == YAML::NodeType::Map) {
        for(size_t i=0; i<node["allowed"].size(); i++) {
            allowedValues <<  ValueWrapper(node["allowed"][i].as<std::string>());
        }
    }
    ValueWrapper defvalue;
    if (node["default"] ) {
        if (std::find(types.begin(), types.end(), SchemaType::Integer) != types.end()) {
            defvalue = ValueWrapper(node["default"].as<int>());
        } else if (std::find(types.begin(), types.end(), SchemaType::Float) != types.end()) {
                  defvalue = ValueWrapper(node["default"].as<double>());
        } else if (std::find(types.begin(), types.end(), SchemaType::String) != types.end()) {
            defvalue = ValueWrapper(node["default"].as<std::string>());
        } else if (std::find(types.begin(), types.end(), SchemaType::Boolean) != types.end()) {
            defvalue = ValueWrapper(node["default"].as<bool>());
        }
    }
    ValueWrapper minvalue;
    if (node["min"] ) {
        if (std::find(types.begin(), types.end(), SchemaType::Integer) != types.end()) {
            minvalue = ValueWrapper(node["min"].as<int>());
        } else if (std::find(types.begin(), types.end(), SchemaType::Float) != types.end()) {
                  minvalue = ValueWrapper(node["min"].as<double>());
        } else {
            minvalue = ValueWrapper();
        }
    }
    ValueWrapper maxvalue;
    if (node["max"] ) {
        if (std::find(types.begin(), types.end(), SchemaType::Integer) != types.end()) {
            maxvalue = ValueWrapper(node["max"].as<int>());
        } else if (std::find(types.begin(), types.end(), SchemaType::Float) != types.end()) {
                  maxvalue = ValueWrapper(node["max"].as<double>());
        } else {
            maxvalue = ValueWrapper();
        }
    }
//    if (!mOrderedKeyList.contains(key))
//        mOrderedKeyList << key;

    bool schemaDefined = (node["schema"] ? true : false);
    mSchemaHelper.insert(key, new Schema(level, node, types, required, allowedValues, defvalue, minvalue, maxvalue, schemaDefined));
    if (schemaDefined) {
        if (mSchemaHelper[key]->hasType(SchemaType::List)) {
            QString str = key + ":-";
            createSchemaHelper(str, node["schema"], ++level);
        } else if (mSchemaHelper[key]->hasType(SchemaType::Dict)) {
                  ++level;
                  QString str;
                  for (YAML::const_iterator it=node["schema"].begin(); it != node["schema"].end(); ++it) {
                      str = key + ":";
                      if (it->second.Type() == YAML::NodeType::Map) {
                          str += QString::fromStdString( it->first.as<std::string>() );
                          createSchemaHelper(str, it->second, level);
                      }
                    }
        } // else  'schema' with niether 'type:list' nor 'type:dict' ?
    }
}

ConnectSchema::ConnectSchema(const QString &inputFileName)
{
    loadFromFile(inputFileName);
}

ConnectSchema::~ConnectSchema()
{
    if (mSchemaHelper.size()>0) {
       qDeleteAll( mSchemaHelper );
       mSchemaHelper.clear();
    }
}

void ConnectSchema::loadFromFile(const QString &inputFileName)
{
    ConnectAgent::loadFromFile(inputFileName);
    if (mRootNode.Type()!=YAML::NodeType::Map)
        EXCEPT() << "Error Loading from file : " << inputFileName;
    for (YAML::const_iterator it = mRootNode.begin(); it != mRootNode.end(); ++it) {
        QString key( QString::fromStdString( it->first.as<std::string>() ) );
        YAML::Node node = it->second;
        if (node["anyof"]) {
            if (node["anyof"].Type()==YAML::NodeType::Sequence) {
                for(size_t i=0; i<node["anyof"].size(); i++) {
                   QString str = QString("%1[%2]").arg(key).arg(i);
                   createSchemaHelper(str, node["anyof"][i], 1);
                }
            }
        } else {
           createSchemaHelper(key, it->second, 1);
        }
    }
}

void ConnectSchema::loadFromString(const QString &input)
{
    ConnectAgent::loadFromString(input);

    for (YAML::const_iterator it = mRootNode.begin(); it != mRootNode.end(); ++it) {
        QString key( QString::fromStdString( it->first.as<std::string>() ) );
        YAML::Node node = it->second;
        createSchemaHelper(key, it->second, 1);
    }

}

QStringList ConnectSchema::getlKeyList() const
{
    return mOrderedKeyList;
}

QStringList ConnectSchema::getFirstLevelKeyList() const
{
    QStringList keyList;
    for(const QString& key : mOrderedKeyList) {
        if (key.contains(":"))
            continue;
        if (key.contains("["))
            keyList << key.left(key.indexOf("["));
        else
            keyList << key;
    }
    return keyList;
}

QStringList ConnectSchema::getNextLevelKeyList(const QString& key) const
{
    QStringList keyList;
    for(const QString& k : mOrderedKeyList) {
        if (k.startsWith(key+":"))
            keyList << k;
    }
    return keyList;
}

QStringList ConnectSchema::getAllRequiredKeyList() const
{
    QStringList keyList;
    for(const QString& key : mSchemaHelper.keys()) {
        if (mSchemaHelper[key]->required)
            keyList << key;
    }
    return keyList;
}

bool ConnectSchema::contains(const QString &key) const
{
    if (mSchemaHelper.contains(key)) {
        return true;
    } else if (isAnyOfDefined(key)) {
        return true;
    } else {
        return false;
    }
}

QStringList ConnectSchema::getAllAnyOfKeys(const QString &key) const
{
    QStringList keyslist;
    for(QMap<QString, Schema*>::const_iterator it= mSchemaHelper.begin(); it!=mSchemaHelper.end(); ++it) {
        if (it.key().contains(key) && !keyslist.contains(it.key()))
            keyslist << it.key();
    }
    return keyslist;

}

int ConnectSchema::getNumberOfAnyOfDefined(const QString &key) const
{
    int num=0;
    QString keystr;
    for(QMap<QString, Schema*>::const_iterator it= mSchemaHelper.begin(); it!=mSchemaHelper.end(); ++it) {
        keystr = QString("%1[%2]").arg(key).arg(num);
        if (keystr.compare(it.key())==0)
            ++num;
    }
    return num;
}

bool ConnectSchema::isAnyOfDefined(const QString &key) const
{
    return (getNumberOfAnyOfDefined(key) > 0);
}

Schema *ConnectSchema::getSchema(const QString &key) const
{
    if (contains(key))
        return mSchemaHelper[key]; //[key];
    else
        return nullptr;
}

QList<SchemaType> ConnectSchema::getType(const QString &key) const
{
    if (contains(key)) {
        return mSchemaHelper[key]->types; // [key]->types;
    } else {
        return QList<SchemaType>();
    }
}

QStringList ConnectSchema::getTypeAsStringList(const QString &key) const
{
    QStringList strlist;
    if (contains(key)) {
        foreach(SchemaType t, mSchemaHelper[key]->types) { // [key]->types) {
            int tt = (int)t;
            if ( tt==(int)SchemaType::Integer)
                strlist << "integer";
            else if ( tt==(int)SchemaType::Float)
                    strlist << "float";
            else if (tt==(int)SchemaType::Boolean)
                     strlist << "boolean";
            else if (tt==(int)SchemaType::String)
                     strlist << "string";
            else if (tt==(int)SchemaType::List)
                     strlist << "list";
            else if (tt==(int)SchemaType::Dict)
                     strlist << "dict";
        }
    }
    return strlist;
}

QStringList ConnectSchema::getAllowedValueAsStringList(const QString &key) const
{
    QStringList strlist;
    if (contains(key)) {
        for(int i=0; i<mSchemaHelper[key]->allowedValues.size(); ++i) {
            int t = (int)mSchemaHelper[key]->allowedValues.at(i).type;
            if ( t==(int)SchemaValueType::Integer)
                strlist << QString::number(mSchemaHelper[key]->allowedValues.at(i).value.intval);
            else if ( t==(int)SchemaValueType::Float)
                    strlist << QString::number(mSchemaHelper[key]->allowedValues.at(i).value.doubleval);
            else if (t==(int)SchemaValueType::String)
                     strlist << QString::fromStdString(mSchemaHelper[key]->allowedValues.at(i).value.stringval);
            else if (t==(int)SchemaValueType::Boolean)
                     strlist << (mSchemaHelper[key]->allowedValues.at(i).value.boolval?"true":"false");
        }
    }
    return strlist;
}

bool ConnectSchema::isRequired(const QString &key) const
{
    if (contains(key)) {
        return mSchemaHelper[key]->required; // [key]->required;
    } else {
        return false;
    }
}

ValueWrapper ConnectSchema::getMin(const QString &key) const
{
    if (contains(key)) {
        return mSchemaHelper[key]->min; // [key]->min;
    } else {
        return ValueWrapper();
    }
}

ValueWrapper ConnectSchema::getMax(const QString &key) const
{
    if (contains(key)) {
        return mSchemaHelper[key]->min; // [key]->max;
    } else {
        return ValueWrapper();
    }
}

bool ConnectSchema::isSchemaDefined(const QString &key) const
{
    if (contains(key))
        return mSchemaHelper[key]->schemaDefined; //[key]->schemaDefined;

    return false;
}

Schema::Schema(int level_, YAML::Node schemaNode_, QList<SchemaType> type_, bool required_, QList<ValueWrapper> allowedValues_, ValueWrapper defaultValue_, ValueWrapper min_, ValueWrapper max_, bool schemaDefined_)
    : level(level_),
      schemaNode(schemaNode_),
      types(type_),
      required(required_),
      allowedValues(allowedValues_),
      defaultValue(defaultValue_),
      min(min_),
      max(max_),
      schemaDefined(schemaDefined_)
{ }


} // namespace connect
} // namespace studio
} // namespace gams
