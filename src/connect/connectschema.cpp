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
#include <QRegularExpression>

#include "connectschema.h"
#include "exception.h"

#include <QRegularExpression>

namespace gams {
namespace studio {
namespace connect {

void ConnectSchema::createSchemaHelper(QString& key, const YAML::Node& node, int level)
{
    if (!mOrderedKeyList.contains(key))
        mOrderedKeyList << key;

    QStringList excludes;
    if (node["excludes"]) {
        mExcludesDefined = true;
        if (node["excludes"].Type()==YAML::NodeType::Sequence) {
            for(size_t i=0; i<node["excludes"].size(); i++) {
                excludes << QString::fromStdString(node["excludes"][i].as<std::string>());
            }
        }
    }
    QList<SchemaType> types;
    if (node["type"]) {
        if (node["type"].Type()==YAML::NodeType::Sequence) {
            for(size_t i=0; i<node["type"].size(); i++) {
                QString value(QString::fromStdString(node["type"][i].as<std::string>()));
                types << getTypeFromValue(value);
            }
        } else if (node["type"].Type()==YAML::NodeType::Scalar) {
                  QString value(QString::fromStdString(node["type"].as<std::string>()));
                  types << getTypeFromValue(value);
        }
    }
    bool nullable = false;
    if (node["nullable"]) {
        if (node["nullable"].Type()==YAML::NodeType::Scalar) {
            nullable = node["nullable"].as<bool>();
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
        if (node["default"].Type()==YAML::NodeType::Null) {
            std::string nullstr("null");
            defvalue = ValueWrapper(nullstr);
        } else {
            if (std::find(types.begin(), types.end(), SchemaType::String) != types.end()) {
                defvalue = ValueWrapper(node["default"].as<std::string>());
            } else if (std::find(types.begin(), types.end(), SchemaType::Integer) != types.end()) {
                defvalue = ValueWrapper(node["default"].as<int>());
            } else if (std::find(types.begin(), types.end(), SchemaType::Float) != types.end()) {
                defvalue = ValueWrapper(node["default"].as<double>());
            } else if (std::find(types.begin(), types.end(), SchemaType::Boolean) != types.end()) {
                defvalue = ValueWrapper(node["default"].as<bool>());
            } else {
                defvalue = ValueWrapper(node["default"].as<std::string>());
            }
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
    mSchemaHelper.insert(key, new Schema(level, node, types, required, nullable, allowedValues, defvalue, minvalue, maxvalue, schemaDefined, excludes));
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
                      } else if (it->second.Type() == YAML::NodeType::Sequence) {
                                 str += QString::fromStdString( it->first.as<std::string>() );
                      }
                    }
        } // else  'schema' with niether 'type:list' nor 'type:dict' ?
    } else if (node["oneof_schema"] ? true : false) {
        if (node["oneof_schema"].Type()==YAML::NodeType::Sequence) {
            ++level;
            for(size_t i=0; i<node["oneof_schema"].size(); i++) {
               QString str = QString("%1:[%2]").arg(key).arg(i);
               if (node["oneof_schema"][i].Type()==YAML::NodeType::Map) {
                   createSchemaHelper(str, node["oneof_schema"][i], level);
                  for (YAML::const_iterator it=node["oneof_schema"][i].begin(); it != node["oneof_schema"][i].end(); ++it) {
                      if (it->second.Type() == YAML::NodeType::Map) {

                          QString nodestr = str + ":" + QString::fromStdString( it->first.as<std::string>() );
                          createSchemaHelper(nodestr, it->second, level+1);
                      }
                 }
               }
            }
        }

    } /*else  if (node["anyof"] ? true : false) {*/
    if (node["anyof"]) {
        if (node["anyof"].Type()==YAML::NodeType::Sequence) {
            if (key.endsWith("-")) {
                key += ":";
            }
            for(size_t i=0; i<node["anyof"].size(); i++) {
               QString str = QString("%1[%2]").arg(key).arg(i);
               createSchemaHelper(str, node["anyof"][i], 1);
            }
        }
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
        createSchemaHelper(key, it->second, 1);
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

const QStringList ConnectSchema::getFirstLevelKeyList() const
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
    int size = key.split(":").size();
    for(const QString& k : mOrderedKeyList) {
        if (k.startsWith(key+":") && k.split(":").size()== size+1) {
            int pos = k.lastIndexOf(QChar('['));
            QString kk = (pos > 0 ? k.left(pos) : k);
            if (!keyList.contains(kk))
                keyList << k;
        }
    }
    return keyList;
}

QStringList ConnectSchema::getAllRequiredKeyList() const
{
    QStringList keyList;
    const QStringList keys = mSchemaHelper.keys();
    for(const QString& key : keys) {
        if (mSchemaHelper[key]->required)
            keyList << key;
    }
    return keyList;
}

bool ConnectSchema::contains(const QString &key) const
{
    if (mSchemaHelper.contains(key)) {
        return true;
    } else {
        return false;
    }
}

QStringList ConnectSchema::getAllLeveledKeys(const QString &key, int level) const
{
    QStringList keyList;
    for(const QString& k : mOrderedKeyList) {
        if ( level!=mSchemaHelper[key]->level)
            continue;
    }
    return keyList;
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
        if (keystr.compare(it.key())==0) {
            ++num;
        } else if (it.key().endsWith(keystr)) {
            ++num;
        }
    }
    return num;
}

bool ConnectSchema::isAnyOfDefined(const QString &key) const
{
    return (getNumberOfAnyOfDefined(key) > 0);
}

QStringList ConnectSchema::getAllOneOfSchemaKeys(const QString &key) const
{
    QStringList keyslist;
    QString pattern = "^"+key+":-:\\[\\d\\d?\\]$";
    QRegularExpression rex(pattern);
    for(QMap<QString, Schema*>::const_iterator it= mSchemaHelper.begin(); it!=mSchemaHelper.end(); ++it) {
        if (rex.match(it.key()).hasMatch() && !keyslist.contains(it.key()))
            keyslist << it.key();
    }
    return keyslist;
}

int ConnectSchema::getNumberOfOneOfSchemaDefined(const QString &key) const
{
    int num=0;
    QString keystr;
    for(QMap<QString, Schema*>::const_iterator it= mSchemaHelper.begin(); it!=mSchemaHelper.end(); ++it) {
        keystr = (!key.endsWith("-") ? QString("%1:-:[%2]").arg(key).arg(num) : QString("%1:[%2]").arg(key).arg(num));
        if (keystr.compare(it.key())==0)
            ++num;
    }
    return num;
}

bool ConnectSchema::isOneOfSchemaDefined(const QString &key) const
{
    return (getNumberOfOneOfSchemaDefined(key) > 0);
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
        for (SchemaType t : std::as_const(mSchemaHelper[key]->types)) {
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

QVariant ConnectSchema::getDefaultValue(const QString &key) const
{
    if (contains(key)) {
        SchemaValueType t = mSchemaHelper[key]->defaultValue.type;
        if (t==SchemaValueType::String) {
            return QVariant(QString::fromStdString(mSchemaHelper[key]->defaultValue.value.stringval));
        } else if ( t==SchemaValueType::Integer) {
                return QVariant(mSchemaHelper[key]->defaultValue.value.intval);
        } else if (t==SchemaValueType::Float) {
                return QVariant(mSchemaHelper[key]->defaultValue.value.doubleval);
        } else if (t==SchemaValueType::Boolean) {
                return QVariant(mSchemaHelper[key]->defaultValue.value.boolval);
        }
    }
    return QVariant();
}

bool ConnectSchema::isNullDefaultAllowed(const QString &key) const
{
    qDebug() << " mSchemaHelper.keys()=" << mSchemaHelper.keys();
    if (contains(key)) {
        qDebug() << " 0";
        if (mSchemaHelper[key]) {
            qDebug() << " 1";
            Value v = mSchemaHelper[key]->defaultValue.value;
            if (strcmp(v.stringval, "null") == 0) {
                qDebug() << " 2";
                return true;
            }
        }
    }
    qDebug() << " 3";
    return false;
}

bool ConnectSchema::isRequired(const QString &key) const
{
    if (contains(key)) {
        if (mSchemaHelper[key]->excludes.isEmpty()) {
            return mSchemaHelper[key]->required;
        }
    }
    return false;
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
        return mSchemaHelper[key]->max; // [key]->max;
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

bool ConnectSchema::isExcludesDefined(const QString &key) const
{
    Q_UNUSED(key)
    return mExcludesDefined;
}

QStringList ConnectSchema::getExcludedKeys(const QString &key) const
{
    if (contains(key))
        return mSchemaHelper[key]->excludes;

    return QStringList();
}

Schema::Schema(int level_, const YAML::Node &schemaNode_, const QList<SchemaType> &type_, bool required_, bool nullable_, const QList<ValueWrapper> &allowedValues_,
               const ValueWrapper &defaultValue_, const ValueWrapper &min_, const ValueWrapper &max_, bool schemaDefined_, const QStringList &excludes_)
    : level(level_),
      schemaNode(schemaNode_),
      types(type_),
      required(required_),
      allowedValues(allowedValues_),
      nullable(nullable_),
      defaultValue(defaultValue_),
      min(min_),
      max(max_),
      schemaDefined(schemaDefined_),
    excludes(excludes_)
{ }


} // namespace connect
} // namespace studio
} // namespace gams
