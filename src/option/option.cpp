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

Option::Option(const QString &systemPath, const QString &optionFileName)
{
    mAvailable = readDefinition(systemPath, optionFileName);
}

Option::~Option()
{
    mOption.clear();
    mSynonymMap.clear();
    mOptionTypeNameMap.clear();
    mOptionGroupList.clear();
}

void Option::dumpAll()
{
    qDebug() << QString("mSynonymMap.size() = %1").arg(mSynonymMap.size());
    QMap<QString, QString>::iterator ssit;
    for (ssit = mSynonymMap.begin(); ssit != mSynonymMap.end(); ++ssit)
        qDebug()  << QString("  [%1] = %2").arg(ssit.key()).arg(ssit.value());

    qDebug() << QString("mOptionGroupList.size() = %1").arg(mOptionGroupList.size());
    for (int i=0; i< mOptionGroupList.size(); ++i) {
        OptionGroup group = mOptionGroupList.at(i);
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
                 if (d != (int)d)
                    return Incorrect_Value_Type;
                 else
                     n = (int)d;
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
        break;
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
   for ( OptionValue value: getValueList(optionName.toUpper()) )
       valueList << value.value.toString();

   return valueList;
}

QStringList Option::getNonHiddenValuesList(const QString &optionName) const
{
    QStringList valueList;
    for ( OptionValue value: getValueList(optionName.toUpper()) ) {
        if (!value.hidden)
           valueList << value.value.toString();
    }

    return valueList;

}

QList<OptionGroup> Option::getOptionGroupList() const
{
    return mOptionGroupList;
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

bool Option::readDefinition(const QString &systemPath, const QString &optionFileName)
{
    if (!CommonPaths::isSystemDirValid())
        return false;
    optHandle_t mOPTHandle;

    optSetExitIndicator(0); // switch of exit() call
    optSetScreenIndicator(0);
    optSetErrorCallback(Option::errorCallback);

    char msg[GMS_SSSIZE];
    optCreateD(&mOPTHandle, systemPath.toLatin1(), msg, sizeof(msg));
    if (msg[0] != '\0') {
        SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
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
             synonym[QString::fromLatin1(name).toUpper()] = QString::fromLatin1(syn).toUpper();
         }

         for (int i=1; i <= optGroupCount(mOPTHandle); ++i) {
             char name[GMS_SSSIZE];
             char help[GMS_SSSIZE];
             int helpContextNr;
             int group;
             optGetGroupNr(mOPTHandle, i, name, &group, &helpContextNr, help);
             mOptionGroupList.append( OptionGroup(name, i, QString::fromLatin1(help), helpContextNr));
         }

         for (int i = 1; i <= optCount(mOPTHandle); ++i) {

                     char name[GMS_SSSIZE];
                     char descript[GMS_SSSIZE];
                     int group;

                     int idefined;
                     int iopttype;
                     int ioptsubtype;
                     int idummy;
                     int irefnr;
                     int itype;

                     optGetHelpNr(mOPTHandle, i, name, descript);
                     optGetInfoNr(mOPTHandle, i, &idefined, &idummy, &irefnr, &itype, &iopttype, &ioptsubtype);

                     QString nameStr = QString::fromLatin1(name).toUpper();
                     OptionDefinition opt(QString::fromLatin1(name), static_cast<optOptionType>(iopttype), static_cast<optDataType>(itype), QString::fromLatin1(descript));

                     int helpContextNr;
                     optGetOptHelpNr(mOPTHandle, i, name, &helpContextNr, &group);
                     opt.groupNumber = group;
                     opt.deprecated = (opt.groupNumber == GAMS_DEPRECATED_GROUP_NUMBER);
                     opt.valid = (helpContextNr == 1);
                     if (synonym.contains(nameStr)) {
                         opt.synonym = synonym[nameStr];
                         mSynonymMap[opt.synonym] = nameStr;
                     }


                     char optTypeName[GMS_SSSIZE];
                     optGetTypeName(mOPTHandle, opt.type, optTypeName);
                     mOptionTypeNameMap[opt.type] = optTypeName;

                     int enumCount = 0;
                     if (iopttype == optTypeInteger) {
                         int iupper;
                         int ilower;
                         int idefval;
                         optGetBoundsInt(mOPTHandle, i, &ilower, &iupper, &idefval);
                         opt.upperBound = QVariant(iupper);
                         opt.lowerBound = QVariant(ilower);
                         opt.defaultValue = QVariant(idefval);
                     } else if (iopttype == optTypeDouble) {
                         double dupper;
                         double dlower;
                         double ddefval;
                         optGetBoundsDbl(mOPTHandle, i, &dlower, &dupper, &ddefval);
                         opt.upperBound = QVariant(dupper);
                         opt.lowerBound = QVariant(dlower);
                         opt.defaultValue = QVariant(ddefval);
                     }
                     switch(iopttype) {
                     case optTypeEnumStr: {
                         int iv;
                         double dv;
                         char sn[GMS_SSSIZE];
                         char sv[GMS_SSSIZE];
                         optGetValuesNr(mOPTHandle, i, sn, &iv, &dv, sv);
                         opt.defaultValue = QVariant(sv);
                         break;
                     }
                     case optTypeEnumInt: {
                         int iv;
                         double dv;
                         char sn[GMS_SSSIZE];
                         char sv[GMS_SSSIZE];
                         optGetValuesNr(mOPTHandle, i, sn, &iv, &dv, sv);
                         opt.defaultValue = QVariant(iv);
                         break;
                     }
//                     case optTypeImmediate: {
//                         char sdefval[GMS_SSSIZE];
//                         optGetDefaultStr(mOPTHandle, i, sdefval);
//                         opt.defaultValue = QVariant(sdefval);
//                         qDebug() << QString("%1, %2").arg(opt.name).arg(sdefval);
//                         break;
//                     }
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

        qDebug() << "Problem reading definition file " << QDir(systemPath).filePath(optionFileName).toLatin1();
        optFree(&mOPTHandle);
        return false;
     }

}

int Option::errorCallback(int count, const char *message)
{
    Q_UNUSED(count);
    auto logger = SysLogLocator::systemLog();
    logger->append(InvalidGAMS, LogMsgType::Error);
    logger->append(message, LogMsgType::Error);
    return 0;
}

} // namespace studio
} // namespace gams
