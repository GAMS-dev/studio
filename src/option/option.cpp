/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "editors/sysloglocator.h"
#include "support/solverconfiginfo.h"

namespace gams {
namespace studio {
namespace option {

QRegularExpression Option::mRexDoubleDash("^[-/][-/](\\S*)$");
QRegularExpression Option::mRexDoubleDashName("^[a-zA-Z]+[_a-zA-Z0-9]*$");
QRegularExpression Option::mRexVersion("^[1-9][0-9](\\.([0-9])(\\.([0-9]))?)?$");
QRegularExpression Option::mRexOptionKey("^([-/]+)");

Option::Option(const QString &optionFilePath, const QString &optionFileName) :
    mOptionDefinitionPath(optionFilePath), mOptionDefinitionFile(optionFileName)
{
    mAvailable = readDefinitionFile(optionFilePath, optionFileName);
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
    QMultiMap<QString, QString>::iterator ssit;
    for (ssit = mSynonymMap.begin(); ssit != mSynonymMap.end(); ++ssit)
        qDebug()  << QString("  [%1] = %2").arg(ssit.key(), ssit.value());

    for( QMap<int, OptionGroup>::const_iterator it=mOptionGroup.cbegin(); it!=mOptionGroup.cend(); ++it) {
        const OptionGroup &group = it.value();
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
        qDebug() << QString(" [%1:%2] %3 [%4] type_%5 %6 range_[%7,%8] group_%9 %10").arg(i++).arg(it.key(),
                            opt.name, opt.synonym, mOptionTypeNameMap[opt.type], opt.description)
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
            qDebug() <<QString("    %1_%2  Hidden:%3 %4").arg(mOptionTypeNameMap[opt.type],
                                                              enumValue.value.toString(),
                                                              enumValue.hidden? "T" : "F",
                                                              enumValue.description);
        }
    }
}

bool Option::isValid(const QString &optionName) const
{
    return (mOption.contains(optionName.toUpper()) && mOption[optionName.toUpper()].valid);
}

bool Option::isSynonymDefined() const
{
    return !mSynonymMap.isEmpty();
}

bool Option::isASynonym(const QString &optionName) const
{
    return mSynonymMap.contains( optionName.toUpper() );
}

bool Option::isDeprecated(const QString &optionName) const
{
    if (mOption.contains(optionName.toUpper())) {
        return mOption[optionName.toUpper()].deprecated;
    } else if (mDeprecatedSynonym.contains(optionName.toUpper())) {
        return true;
    }

    return false;
}

bool Option::isDoubleDashedOption(const QString &option) const
{
    return mRexDoubleDash.match(option).hasMatch();
}

bool Option::isDoubleDashedOptionNameValid(const QString &optionName) const
{
    return mRexDoubleDashName.match(optionName).hasMatch();
}

bool Option::isConformantVersion(const QString &version) const
{
    return mRexVersion.match(version).hasMatch();
}

OptionErrorType Option::getValueErrorType(const QString &optionName, const QString &value) const
{
    QString key = optionName;
    if (!isValid(key)) {
        if (isASynonym(key))
            key = getNameFromSynonym(optionName);
        else
            return OptionErrorType::Invalid_Key;
    }

    if (isDeprecated(key))
        return OptionErrorType::Deprecated_Option;

    if (value.simplified().isEmpty())
        return OptionErrorType::Missing_Value;

    switch(getOptionType(key)) {
     case   optTypeImmediate : {
        return OptionErrorType::No_Error;
     }
     case optTypeEnumInt : {
         bool isCorrectDataType = false;
         int n = value.toInt(&isCorrectDataType);
         if (isCorrectDataType) {
            for (const OptionValue &optValue: getValueList(key)) {
               if (optValue.value.toInt() == n) { // && !optValue.hidden) {
                   return OptionErrorType::No_Error;
               }
            }
            return OptionErrorType::Value_Out_Of_Range;
         } else {
             return OptionErrorType::Incorrect_Value_Type;
         }
     }
     case optTypeEnumStr : {
         for (const OptionValue &optValue: getValueList(key)) {
           if (QString::compare(optValue.value.toString(), value, Qt::CaseInsensitive)==0) { //&& !optValue.hidden) {
               return OptionErrorType::No_Error;
           }
         }
        return OptionErrorType::Value_Out_Of_Range;
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
                    return OptionErrorType::Incorrect_Value_Type;

                 bool isCorrectDataType = false;
                 double d = value.toDouble(&isCorrectDataType);
                 int ivalue = static_cast<int>(d);
                 if (qAbs(d-ivalue) < 0.01)
                     n = ivalue;
                 else
                    return OptionErrorType::Incorrect_Value_Type;
             }
          }
        }
        if ((n < getLowerBound(key).toInt()) || (getUpperBound(key).toInt() < n))
            return OptionErrorType::Value_Out_Of_Range;
        else
             return OptionErrorType::No_Error;
    }
    case optTypeDouble: {
        bool isCorrectDataType = false;
        double d = value.toDouble(&isCorrectDataType);
        if (!isCorrectDataType) {
           if (value.compare("eps", Qt::CaseInsensitive)==0) {
               return OptionErrorType::No_Error;
           } else if (value.compare("maxdouble", Qt::CaseInsensitive)==0) {
                      d = OPTION_VALUE_MAXDOUBLE;
           } else if (value.compare("mindouble", Qt::CaseInsensitive)==0) {
                     d = OPTION_VALUE_MINDOUBLE;
           } else {
                QDoubleValidator doublev(getLowerBound(key).toDouble(), getUpperBound(key).toDouble(), OPTION_VALUE_DECIMALS);
                QString v = value;
                int pos = 0;
                if (doublev.validate(v, pos) != QValidator::Acceptable)
                   return OptionErrorType::Incorrect_Value_Type;
            }
        }
        if ((d < getLowerBound(key).toDouble()) || (getUpperBound(key).toDouble() < d))
            return OptionErrorType::Value_Out_Of_Range;
        else
            return OptionErrorType::No_Error;
     }
    case optTypeBoolean: {
        bool isCorrectDataType = false;
        int n = value.toInt(&isCorrectDataType);
        if (isCorrectDataType) {
            if (n==0 || n==1) {
                return OptionErrorType::No_Error;
            }
        }
        return OptionErrorType::Incorrect_Value_Type;
    }
    default:
        break;
    }
    return OptionErrorType::No_Error;  //Unknown_Error;
}

QString Option::getNameFromSynonym(const QString &synonym) const
{
    return mSynonymMap.value(synonym.toUpper());
}

optOptionType Option::getOptionType(const QString &optionName) const
{
    return mOption[optionName.toUpper()].type;
}

optOptionSubType Option::getOptionSubType(const QString &optionName) const
{
    return mOption[optionName.toUpper()].subType;
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

const QList<OptionValue> Option::getValueList(const QString &optionName) const
{
    return mOption[optionName.toUpper()].valueList;
}

const QString Option::getEOLChars() const
{
    return mEOLChars;
}

bool Option::isEOLCharDefined() const
{
    return !mEOLChars.isEmpty();
}

QString Option::getDefaultSeparator() const
{
    return mSeparator;
}

bool Option::isDefaultSeparatorDefined() const
{
    return !mSeparator.isEmpty();
}

QString Option::getDefaultStringquote() const
{
    return mStringquote;
}

bool Option::isDefaultStringquoteDefined() const
{
    return !mStringquote.isEmpty();
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
    return getKeyList();
}

QStringList Option::getValuesList(const QString &optionName) const
{
   QStringList valueList;
   for ( const OptionValue &value: getValueList(optionName.toUpper()) )
       valueList << value.value.toString();

   return valueList;
}

const QStringList Option::getSynonymList(const QString &optionName) const
{
    QStringList synonymList;
    if (mSynonymMap.contains(optionName.toUpper())) {
        synonymList = mSynonymMap.keys( optionName.toUpper() );
    }
    return synonymList;
}

QStringList Option::getNonHiddenValuesList(const QString &optionName) const
{
    QStringList valueList;
    for ( const OptionValue &value: getValueList(optionName.toUpper()) ) {
        if (!value.hidden)
           valueList << value.value.toString();
    }

    return valueList;

}

int Option::getOrdinalNumber(const QString &optionName) const
{
    if (isValid(optionName))
        return mOption[optionName.toUpper()].number;
    else
        return -1;
}

int Option::getGroupNumber(const QString &optionName) const
{
    return mOption[optionName.toUpper()].groupNumber;
}

bool Option::isGroupHidden(int number) const
{
    return mOptionGroup[number].hidden;
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

QString Option::getOptionKey(const QString &option) const
{
    QRegularExpressionMatch match = mRexOptionKey.match(option);
    if (match.hasMatch())
       return QString(option.mid(match.capturedLength()));
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

bool Option::isModified(const QString &optionName) const
{
    return mOption[optionName.toUpper()].modified;
}

void Option::setModified(const QString &optionName, bool modified)
{
    mOption[optionName.toUpper()].modified = modified;
}

void Option::resetModficationFlag()
{
    for( QMap<QString, OptionDefinition>::iterator it=mOption.begin(); it!=mOption.end(); ++it) {
        it.value().modified = false;
    }
}

QString Option::getOptionDefinitionFile() const
{
    return mOptionDefinitionFile;
}

QString Option::getOptionDefinitionPath() const
{
    return mOptionDefinitionPath;
}

OptionDefinition Option::getOptionDefinition(const QString &optionName) const
{
    return mOption[optionName.toUpper()];
}

bool Option::readDefinitionFile(const QString &optionFilePath, const QString &optionFileName)
{
    if (!CommonPaths::isSystemDirValid())
        return false;

    optSetExitIndicator(0); // switch of exit() call
    optSetScreenIndicator(0);
    optSetErrorCallback(Option::errorCallback);

    optHandle_t mOPTHandle;

    char msg[GMS_SSSIZE];
    if (!optCreateD(&mOPTHandle, CommonPaths::systemDir().toLatin1(), msg, sizeof(msg)))
       return false;

    if (msg[0] != '\0') {
        SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
        return false;
    }

    gams::studio::support::SolverConfigInfo solverConfigInfo;
    QMap<QString, int> modeltypes;
    for (int i=1; i<solverConfigInfo.modelTypes(); ++i) {
        modeltypes[solverConfigInfo.modelTypeName(i)] = i;
    }
    if (!optReadDefinition(mOPTHandle, QDir(optionFilePath).filePath(optionFileName).toLatin1())) {

         QMultiMap<QString, QString> synonym;
         char name[GMS_SSSIZE];
         char syn[GMS_SSSIZE];
         for (int i = 1; i <= optSynonymCount(mOPTHandle); ++i) {
             optGetSynonym(mOPTHandle, i, syn, name);
             synonym.insert(QString::fromLatin1(name), QString::fromLatin1(syn));
             if (optIsDeprecated(mOPTHandle, syn))
                mDeprecatedSynonym << QString::fromLatin1(syn).toUpper();
         }

         for (int i=1; i <= optGroupCount(mOPTHandle); ++i) {
             char name[GMS_SSSIZE];
             char help[GMS_SSSIZE];
             int helpContextNr;
             int group;
             optGetGroupNr(mOPTHandle, i, name, &group, &helpContextNr, help);
             mOptionGroup.insert(i, OptionGroup(name, i, (helpContextNr==0), QString::fromLatin1(help), helpContextNr));
         }

         char eolchars[GMS_SSSIZE];
         int numChars = optEOLChars(mOPTHandle, eolchars);
         mEOLChars = (numChars>0) ?  QString(eolchars) : "";

         char separatorChars[GMS_SSSIZE];
         char* c = optSeparator(mOPTHandle, separatorChars);
         mSeparator = (c ? (c[0] ? separatorChars : " ") : " ");

         char stringquoteChars[GMS_SSSIZE];
         char* s = optStringQuote(mOPTHandle, stringquoteChars);
         mStringquote = (s ? (s[0] ? stringquoteChars : "") : "");

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

             int ivalue;
             double dvalue;
             char svalue[GMS_SSSIZE];

             int helpContextNr;

             optGetHelpNr(mOPTHandle, i, name, descript);
             optGetInfoNr(mOPTHandle, i, &idefined, &idummy, &irefnr, &itype, &iopttype, &ioptsubtype);
             optGetOptHelpNr(mOPTHandle, i, name, &helpContextNr, &group);
             optGetValuesNr(mOPTHandle, i, name, &ivalue, &dvalue, svalue);

             QString nameStr = QString::fromLatin1(name);
             OptionDefinition opt(i, QString::fromLatin1(name),
                                  static_cast<optDataType>(itype),
                                  static_cast<optOptionType>(iopttype),
                                  static_cast<optOptionSubType>(ioptsubtype),
                                  QString::fromLatin1(descript));

             opt.groupNumber = group;
             /* only for gams "solver" option */
             if (mOptionDefinitionFile.compare("optgams.def", Qt::CaseInsensitive)==0 &&
                 opt.name.compare("solver", Qt::CaseInsensitive)==0                      ) {
                const auto names = solverConfigInfo.solverNames();
                for(const QString &s : names) {
                   opt.valueList.append( OptionValue( QVariant(s), QString(), false, false));
                }
             }
             opt.deprecated = optIsDeprecated(mOPTHandle, name);
             opt.valid = (helpContextNr != 0);
             QStringList synonymList;
             if (synonym.contains(nameStr)) {
                 QMultiMap<QString, QString>::iterator it = synonym.find(nameStr);
                 while (it != synonym.end() && (QString::compare(it.key(), nameStr, Qt::CaseInsensitive) == 0) ) {
                       if (!isDeprecated(it.value()))
                          synonymList << it.value();
                       mSynonymMap.insert(it.value().toUpper(), it.key());
                       ++it;
                 }

             }
             if (!synonymList.isEmpty())
                 opt.synonym = synonymList.join(",");
             else
                 opt.synonym = "";

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
                  opt.defaultValue = QVariant(ivalue);
                  break;
             }
             case optTypeString:
                 char sdefval[GMS_SSSIZE];
                 optGetDefaultStr(mOPTHandle, i, sdefval);
                 opt.defaultValue = QString(sdefval);
                 if (modeltypes.contains(nameStr)) {
                     opt.defaultValue = solverConfigInfo.defaultSolverFormodelTypeName( modeltypes.value(nameStr) );
                     QStringList solvers = solverConfigInfo.solversForModelType( modeltypes[nameStr] );
                     for (const QString &s : std::as_const(solvers)) {
                         opt.valueList.append( OptionValue(QVariant(s), "", false, false ) );
                     }
                 }
                 break;
             case optTypeStrList :
             case optTypeEnumStr: {
//                     case optTypeImmediate: {
                 char sdefval[GMS_SSSIZE];
                 optGetDefaultStr(mOPTHandle, i, sdefval);
                 opt.defaultValue = QString(sdefval);
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
             mOption[nameStr.toUpper()] = opt;
         }
         optFree(&mOPTHandle);
         return true;
     } else {

        SysLogLocator::systemLog()->append( QString("Problem reading definition file: %1").arg(QDir(CommonPaths::systemDir()).filePath(optionFileName)), LogMsgType::Error);
        qDebug() << "Problem reading definition file " << QDir(CommonPaths::systemDir()).filePath(optionFileName).toLatin1();
        optFree(&mOPTHandle);
        return false;
     }

}

int Option::errorCallback(int count, const char *message)
{
    Q_UNUSED(count)
    auto logger = SysLogLocator::systemLog();
    logger->append(InvalidGAMS, LogMsgType::Error);
    logger->append(message, LogMsgType::Error);
    return 0;
}

} // namespace option
} // namespace studio
} // namespace gams
