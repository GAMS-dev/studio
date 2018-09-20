/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include <QIntValidator>
#include <QDoubleValidator>
#include <QDir>
#include "exception.h"
#include "editors/systemlogedit.h"
#include "gclgms.h"
#include "option.h"
#include "commonpaths.h"
#include "locators/sysloglocator.h"

namespace gams {
namespace studio {

Option::Option(const QString &systemPath, const QString &optionFileName) :
    mOptionDefinitionPath(systemPath), mOptionDefinitionFile(optionFileName)
{
    mAvailable = readDefinitionFile(systemPath, optionFileName);
}

Option::~Option()
{
    mOption.clear();
    mSynonymMap.clear();
    mOptionTypeNameMap.clear();
    mOptionGroup.clear();
}

void Option::dumpAll()
{
    qDebug() << QString("mSynonymMap.size() = %1").arg(mSynonymMap.size());
    QMap<QString, QString>::iterator ssit;
    for (ssit = mSynonymMap.begin(); ssit != mSynonymMap.end(); ++ssit)
        qDebug()  << QString("  [%1] = %2").arg(ssit.key()).arg(ssit.value());

    for( QMap<int, OptionGroup>::const_iterator it=mOptionGroup.cbegin(); it!=mOptionGroup.cend(); ++it) {
        OptionGroup group = it.value();
        qDebug() << QString("%1: %2 %3 help_%4 %5").arg(group.number).arg(group.name).arg(group.helpContext).arg(group.description);
    }

    qDebug() << QString("mOptionTypeNameMap.size() = %1").arg(mOptionTypeNameMap.size());
    for( QMap<int, QString>::const_iterator it=mOptionTypeNameMap.cbegin(); it!=mOptionTypeNameMap.cend(); ++it) {
        qDebug() << QString("%1: %2").arg(it.key()).arg(it.value());
    }

    qDebug() << QString("mOption.size() = %1").arg(mOption.size());
    int i = 0;
    for( QMap<QString, OptionDefinition>::const_iterator it=mOption.cbegin(); it!=mOption.cend(); ++it) {
        OptionDefinition opt = it.value();
        qDebug() << QString(" [%1:%2] %3 [%4] type_%5 %6 range_[%7,%8] group_%9 %10").arg(i++).arg(it.key())
                            .arg(opt.name).arg(opt.synonym).arg(mOptionTypeNameMap[opt.type]).arg(opt.description)
                            .arg( opt.lowerBound.canConvert<int>() ? opt.lowerBound.toInt() : opt.lowerBound.toDouble() )
                            .arg( opt.upperBound.canConvert<int>() ? opt.upperBound.toInt() : opt.upperBound.toDouble() )
                            .arg( opt.groupNumber ).arg( opt.valid ? "SHOWN": "HIDDEN");
        switch(opt.dataType) {
             case optDataInteger:
                   qDebug() << QString("  default_%1").arg( opt.defaultValue.toInt() );
                   break;
             case optDataDouble:
                   qDebug() << QString("  default_%1").arg( opt.defaultValue.toDouble() );
                   break;
             case optDataString:
             default:
                   qDebug() << QString("  default_") << opt.defaultValue.toString();
                   break;
        }
        for(int j =0; j< opt.valueList.size(); j++) {
            OptionValue enumValue = opt.valueList.at(j);
            qDebug() <<QString("    %1_%2  Hidden:%3 %4").arg(mOptionTypeNameMap[opt.type]).arg(enumValue.value.toString()).arg(enumValue.hidden? "T" : "F").arg(enumValue.description);
        }
    }
}

bool Option::isValid(const QString &optionName) const
{
    return mOption.contains(optionName.toUpper());
}

bool Option::isASynonym(const QString &optionName) const
{
    return mSynonymMap.contains( optionName.toUpper() );
}

bool Option::isDeprecated(const QString &optionName) const
{
    if (isValid(optionName))
       return (mOption[optionName.toUpper()].groupNumber == GAMS_DEPRECATED_GROUP_NUMBER);

    return false;
}

bool Option::isDoubleDashedOption(const QString &option) const
{
    return QRegExp("^[-/][-/](\\S*)").exactMatch(option);
}

bool Option::isDoubleDashedOptionNameValid(const QString &optionName) const
{
    return QRegExp("^[a-zA-Z]+[_a-zA-Z0-9]*").exactMatch(optionName) ;
}

OptionErrorType Option::getValueErrorType(const QString &optionName, const QString &value) const
{
    QString key = optionName;
    if (!isValid(key))
        key = getNameFromSynonym(optionName);

    switch(getOptionType(key)) {
     case optTypeEnumInt : {
         bool isCorrectDataType = false;
         int n = value.toInt(&isCorrectDataType);
         if (isCorrectDataType) {
            for (OptionValue optValue: getValueList(key)) {
               if (optValue.value.toInt() == n) { // && !optValue.hidden) {
                   return No_Error;
               }
            }
            return Value_Out_Of_Range;
         } else {
             return Incorrect_Value_Type;
         }
     }
     case optTypeEnumStr : {
         for (OptionValue optValue: getValueList(key)) {
           if (QString::compare(optValue.value.toString(), value, Qt::CaseInsensitive)==0) { //&& !optValue.hidden) {
               return No_Error;
           }
         }
        return Value_Out_Of_Range;
     }
     case optTypeInteger: {
        bool isCorrectDataType = false;
        int n = value.toInt(&isCorrectDataType);
        if (!isCorrectDataType) {
           if (value.compare("maxint", Qt::CaseInsensitive)==0) {
               n = OPTION_VALUE_MAXINT;
           } else if (value.compare("minint", Qt::CaseInsensitive)==0) {
                     n = OPTION_VALUE_MININT;
           } else {
               QIntValidator intv(getLowerBound(key).toInt(), getUpperBound(key).toInt());
               QString v = value;
              int pos = 0;
              if (intv.validate(v, pos) != QValidator::Acceptable) {
                 QDoubleValidator doublev;
                 doublev.setNotation(QDoubleValidator::ScientificNotation);
                 doublev.setDecimals(OPTION_VALUE_DECIMALS);
                 doublev.setTop(GMS_SV_PINF);
                 if (doublev.validate(v, pos) != QValidator::Acceptable)
                    return Incorrect_Value_Type;

                 bool isCorrectDataType = false;
                 double d = value.toDouble(&isCorrectDataType);
                 int ivalue = static_cast<int>(d);
                 if (std::abs(d-ivalue) < 0.01)
                     n = ivalue;
                 else
                    return Incorrect_Value_Type;
             }
          }
        }
        if ((n < getLowerBound(key).toInt()) || (getUpperBound(key).toInt() < n))
            return Value_Out_Of_Range;
        else
             return No_Error;
    }
    case optTypeDouble: {
        bool isCorrectDataType = false;
        double d = value.toDouble(&isCorrectDataType);
        if (!isCorrectDataType) {
           if (value.compare("eps", Qt::CaseInsensitive)==0) {
               return No_Error;
           } else if (value.compare("maxdouble", Qt::CaseInsensitive)==0) {
                      d = OPTION_VALUE_MAXDOUBLE;
           } else if (value.compare("mindouble", Qt::CaseInsensitive)==0) {
                     d = -OPTION_VALUE_MINDOUBLE;
           } else {
                QDoubleValidator doublev(getLowerBound(key).toDouble(), getUpperBound(key).toDouble(), OPTION_VALUE_DECIMALS);
                QString v = value;
                int pos = 0;
                if (doublev.validate(v, pos) != QValidator::Acceptable)
                   return Incorrect_Value_Type;
            }
        }
        if ((d < getLowerBound(key).toDouble()) || (getUpperBound(key).toDouble() < d))
            return Value_Out_Of_Range;
        else
            return No_Error;
     }
     default:
        break;
    }
    return No_Error;  //Unknown_Error;
}

QString Option::getNameFromSynonym(const QString &synonym) const
{
    return mSynonymMap[synonym.toUpper()];
}

optOptionType Option::getOptionType(const QString &optionName) const
{
    return mOption[optionName.toUpper()].type;
}

optDataType Option::getDataType(const QString &optionName) const
{
    return mOption[optionName.toUpper()].dataType;
}

QVariant Option::getUpperBound(const QString &optionName) const
{
    return mOption[optionName.toUpper()].upperBound;
}

QVariant Option::getLowerBound(const QString &optionName) const
{
    return mOption[optionName.toUpper()].lowerBound;
}

QVariant Option::getDefaultValue(const QString &optionName) const
{
    return mOption[optionName.toUpper()].defaultValue;
}

QString Option::getDescription(const QString &optionName) const
{
    return mOption[optionName.toUpper()].description;
}

QList<OptionValue> Option::getValueList(const QString &optionName) const
{
    return mOption[optionName.toUpper()].valueList;
}

QStringList Option::getKeyList() const
{
    QStringList keyList;
    for( QMap<QString, OptionDefinition>::const_iterator it=mOption.cbegin(); it!=mOption.cend(); ++it) {
        keyList << it.value().name;
    }
    return keyList;
}

QStringList Option::getValidNonDeprecatedKeyList() const
{
    QStringList keyList;
    for( QMap<QString, OptionDefinition>::const_iterator it=mOption.cbegin(); it!=mOption.cend(); ++it) {
        if (isDeprecated(it.key()))
            continue;
        if (!it.value().valid)
            continue;
        keyList << it.value().name;
    }
    return keyList;
}

QStringList Option::getKeyAndSynonymList() const
{
    // TODO
    return getKeyList();
}

QStringList Option::getValuesList(const QString &optionName) const
{
   QStringList valueList;
   foreach( OptionValue value, getValueList(optionName.toUpper()) )
       valueList << value.value.toString();

   return valueList;
}

QStringList Option::getNonHiddenValuesList(const QString &optionName) const
{
    QStringList valueList;
    foreach( OptionValue value, getValueList(optionName.toUpper()) ) {
        if (!value.hidden)
           valueList << value.value.toString();
    }

    return valueList;

}

int Option::getOrdinalNumber(const QString &optionName) const
{
    return mOption[optionName.toUpper()].number;
}

int Option::getGroupNumber(const QString &optionName) const
{
    return mOption[optionName.toUpper()].groupNumber;
}

QString Option::getGroupName(const QString &optionName) const
{
    return mOptionGroup[getGroupNumber(optionName)].name;
}

QString Option::getGroupDescription(const QString &optionName) const
{
    return mOptionGroup[getGroupNumber(optionName)].description;
}

QList<OptionGroup> Option::getOptionGroupList() const
{
    return mOptionGroup.values();
}

QString Option::getOptionTypeName(int type) const
{
    return mOptionTypeNameMap[type];
}

QString Option::getOptionKey(const QString &option)
{
    QRegExp regexp("^([-/]+)");
    int pos = regexp.indexIn(option);
    if (pos != -1)
       return QString(option.mid(regexp.matchedLength()));
    else
       return QString(option);
}

bool Option::available() const
{
    return mAvailable;
}

QMap<QString, OptionDefinition> Option::getOption() const
{
    return mOption;
}

OptionDefinition Option::getOptionDefinition(const QString &optionName) const
{
    return mOption[optionName.toUpper()];
}

bool Option::readDefinitionFile(const QString &systemPath, const QString &optionFileName)
{
    if (!CommonPaths::isSystemDirValid())
        return false;
    optHandle_t mOPTHandle;

    char msg[GMS_SSSIZE];
    optCreateD(&mOPTHandle, systemPath.toLatin1(), msg, sizeof(msg));
    if (msg[0] != '\0') {
        SysLogLocator::systemLog()->appendLog(msg, LogMsgType::Error);
        qDebug() << QString("ERROR: ").arg(msg);
        optFree(&mOPTHandle);
        return false;
    }

    if (!optReadDefinition(mOPTHandle, QDir(systemPath).filePath(optionFileName).toLatin1())) {

         QMap<QString, QString> synonym;
         char name[GMS_SSSIZE];
         char syn[GMS_SSSIZE];
         for (int i = 1; i <= optSynonymCount(mOPTHandle); ++i) {
             optGetSynonym(mOPTHandle, i, syn, name);
             synonym.insertMulti(QString::fromLatin1(name).toUpper(), QString::fromLatin1(syn).toUpper());
         }

         for (int i=1; i <= optGroupCount(mOPTHandle); ++i) {
             char name[GMS_SSSIZE];
             char help[GMS_SSSIZE];
             int helpContextNr;
             int group;
             optGetGroupNr(mOPTHandle, i, name, &group, &helpContextNr, help);
             mOptionGroup.insert(i, OptionGroup(name, i, QString::fromLatin1(help), helpContextNr));
         }

         for (int i = 1; i <= optCount(mOPTHandle); ++i) {

             char name[GMS_SSSIZE];
             char descript[GMS_SSSIZE];
             int group = 0;

             int idefined;
             int iopttype;
             int ioptsubtype;
             int idummy;
             int irefnr;
             int itype;

             optGetHelpNr(mOPTHandle, i, name, descript);
             optGetInfoNr(mOPTHandle, i, &idefined, &idummy, &irefnr, &itype, &iopttype, &ioptsubtype);

             QString nameStr = QString::fromLatin1(name).toUpper();
             OptionDefinition opt(i, QString::fromLatin1(name), static_cast<optOptionType>(iopttype), static_cast<optDataType>(itype), QString::fromLatin1(descript));

             int helpContextNr;
             optGetOptHelpNr(mOPTHandle, i, name, &helpContextNr, &group);
             opt.groupNumber = group;
             opt.deprecated = (opt.groupNumber == GAMS_DEPRECATED_GROUP_NUMBER);
             opt.valid = (helpContextNr == 1);
             if (synonym.contains(nameStr)) {
                 QMap<QString, QString>::const_iterator it = synonym.find(nameStr);
                 while (it != synonym.end() && (QString::compare(it.key(), nameStr, Qt::CaseInsensitive) == 0) ) {
                       opt.synonym = it.value();
                       mSynonymMap.insertMulti(it.value(), it.key());
                       ++it;
                 }
             }

             char optTypeName[GMS_SSSIZE];
             optGetTypeName(mOPTHandle, opt.type, optTypeName);
             mOptionTypeNameMap[opt.type] = optTypeName;

             int enumCount = 0;
             switch(iopttype) {
             case optTypeInteger: {
                         int iupper;
                         int ilower;
                         int idefval;
                         optGetBoundsInt(mOPTHandle, i, &ilower, &iupper, &idefval);
                         opt.upperBound = QVariant(iupper);
                         opt.lowerBound = QVariant(ilower);
                         opt.defaultValue = QVariant(idefval);
                         break;
                     }
             case optTypeDouble: {
                         double dupper;
                         double dlower;
                         double ddefval;
                         optGetBoundsDbl(mOPTHandle, i, &dlower, &dupper, &ddefval);
                         opt.upperBound = QVariant(dupper);
                         opt.lowerBound = QVariant(dlower);
                         opt.defaultValue = QVariant(ddefval);
                         break;
                     }
             case optTypeEnumInt:
             case optTypeBoolean: {
                         int iv;
                         double dv;
                         char sn[GMS_SSSIZE];
                         char sv[GMS_SSSIZE];
                         optGetValuesNr(mOPTHandle, i, sn, &iv, &dv, sv);
                         opt.defaultValue = QVariant(iv);
                         break;
                     }
             case optTypeString:
             case optTypeStrList :
             case optTypeEnumStr: {
//                     case optTypeImmediate: {
                 char sdefval[GMS_SSSIZE];
                 optGetDefaultStr(mOPTHandle, i, sdefval);
                 opt.defaultValue = QVariant(sdefval);
                 break;
            }
             default: break;
             }

             optGetEnumCount(mOPTHandle, i, &enumCount);
             for (int j = 1; j <= enumCount; j++) {
                  int ihelpContext;
                  char shelpText[GMS_SSSIZE];
                  optGetEnumHelp(mOPTHandle, i, j, &ihelpContext, shelpText);
                  switch( itype ) {
                  case optDataString:
                              int ipos;
                              int idummy;
                              char sdefval[GMS_SSSIZE];
                              char soptvalue[GMS_SSSIZE];
                              optGetEnumStrNr(mOPTHandle, i, sdefval, &ipos);
                              optGetEnumValue(mOPTHandle, i, j, &idummy, soptvalue);
                              opt.defaultValue = QVariant(QString::fromLatin1(sdefval));
                              opt.valueList.append( OptionValue(QVariant(QString::fromLatin1(soptvalue)), QString::fromLatin1(shelpText), (ihelpContext==0), (iopttype==optTypeEnumInt || iopttype==optTypeEnumStr)) );
                              break;
                  case optDataInteger:
                              int ioptvalue;
                              char sdummy[GMS_SSSIZE];
                              optGetEnumValue(mOPTHandle, i, j, &ioptvalue, sdummy);
                              opt.valueList.append( OptionValue(QVariant(ioptvalue), QString::fromLatin1(shelpText), (ihelpContext==0), (iopttype==optTypeEnumInt || iopttype==optTypeEnumStr)) );
                              break;
                  case optDataDouble:
                  default:
                          break;
                  }
             }
             mOption[nameStr] = opt;
         }
         optFree(&mOPTHandle);
         return true;
     } else {

        SysLogLocator::systemLog()->appendLog( QString("Problem reading definition file: %1").arg(QDir(systemPath).filePath(optionFileName)), LogMsgType::Error);
        qDebug() << "Problem reading definition file " << QDir(systemPath).filePath(optionFileName).toLatin1();
        optFree(&mOPTHandle);
        return false;
     }

}

} // namespace studio
} // namespace gams
