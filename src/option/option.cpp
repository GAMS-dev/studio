#include "exception.h"
#include "gclgms.h"
#include "option.h"

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
        qDebug() << QString("%1: %2 %3 help_%4").arg(group.number).arg(group.name).arg(group.helpContext).arg(group.description);
    }

    qDebug() << QString("mOptionTypeNameMap.size() = %1").arg(mOptionTypeNameMap.size());
    for( QMap<int, QString>::const_iterator it=mOptionTypeNameMap.cbegin(); it!=mOptionTypeNameMap.cend(); ++it) {
        qDebug() << QString("%1: %2").arg(it.key()).arg(it.value());
    }

    qDebug() << QString("mOption.size() = %1").arg(mOption.size());
    int i = 0;
    for( QMap<QString, OptionDefinition>::const_iterator it=mOption.cbegin(); it!=mOption.cend(); ++it) {
        OptionDefinition opt = it.value();
        qDebug() << QString(" [%1:%2] %3 [%4] type_%5 %6 range_[%7,%8] group_%9").arg(i++).arg(it.key())
                            .arg(opt.name).arg(opt.synonym).arg(mOptionTypeNameMap[opt.type]).arg(opt.description)
                            .arg( opt.lowerBound.canConvert<int>() ? opt.lowerBound.toInt() : opt.lowerBound.toDouble() )
                            .arg( opt.upperBound.canConvert<int>() ? opt.upperBound.toInt() : opt.upperBound.toDouble() )
                            .arg( opt.groupNumber );
        switch(opt.dataType) {
             case optDataInteger:
                      qDebug() << QString("  default_%1").arg( opt.defaultValue.toInt() );
                       break;
             case optDataDouble:
                       qDebug() << QString("  default_%1").arg( opt.defaultValue.toDouble() );
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

bool Option::isValid(const QString &optionName)
{
    return mOption.contains(optionName.toUpper());
}

bool Option::isThereASynonym(const QString &optionName)
{
    return mSynonymMap.contains( optionName.toUpper() );
}

bool Option::isDeprecated(const QString &optionName)
{
    if (isValid(optionName))
       return (mOption[optionName.toUpper()].groupNumber == GAMS_DEPRECATED_GROUP_NUMBER);

    return false;
}

bool Option::isDoubleDashedOption(const QString &optionName)
{
    return (optionName.startsWith("--") || optionName.startsWith("-/") || optionName.startsWith("/-") || optionName.startsWith("//") );
}

QString Option::getSynonym(const QString &optionName) const
{
    return mSynonymMap[optionName.toUpper()];
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

QList<OptionGroup> Option::getOptionGroupList() const
{
    return mOptionGroupList;
}

QString Option::getOptionTypeName(int type) const
{
    return mOptionTypeNameMap[type];
}

bool Option::available() const
{
    return mAvailable;
}

OptionDefinition Option::getOptionDefinition(const QString &optionName) const
{
    return mOption[optionName.toUpper()];
}

bool Option::readDefinition(const QString &systemPath, const QString &optionFileName)
{
    optHandle_t mOPTHandle;

    char msg[GMS_SSSIZE];
    optCreateD(&mOPTHandle, systemPath.toLatin1(), msg, sizeof(msg));
    if (msg[0] != '\0') {
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

                     if (synonym.contains(nameStr)) {
                         opt.synonym = synonym[nameStr];
                         mSynonymMap[opt.synonym] = nameStr;
                     }
                     char optTypeName[GMS_SSSIZE];
                     optGetTypeName(mOPTHandle, opt.type, optTypeName);
                     mOptionTypeNameMap[opt.type] = optTypeName;

                     int enumCount = 0;
                     switch(iopttype) {
                       case optTypeInteger:
                            int iupper;
                            int ilower;
                            int idefval;
                            optGetBoundsInt(mOPTHandle, i, &ilower, &iupper, &idefval);
                            opt.upperBound = QVariant(iupper);
                            opt.lowerBound = QVariant(ilower);
                            opt.defaultValue = QVariant(idefval);
                            break;
                       case optTypeDouble:
                            double dupper;
                            double dlower;
                            double ddefval;
                            optGetBoundsDbl(mOPTHandle, i, &dlower, &dupper, &ddefval);
                            opt.upperBound = QVariant(dupper);
                            opt.lowerBound = QVariant(dlower);
                            opt.defaultValue = QVariant(ddefval);
                           break;
                       case optTypeString:
                            char sdefval[GMS_SSSIZE];
                            optGetDefaultStr(mOPTHandle, i, sdefval);
                            opt.defaultValue = QVariant(sdefval);
                            break;
                       case optTypeBoolean:
                       case optTypeEnumStr:
                       case optTypeEnumInt:
                       case optTypeMultiList:
                       case optTypeStrList:
                       case optTypeMacro:
                         break;
                       case optTypeImmediate:
                         optGetDefaultStr(mOPTHandle, i, sdefval);
                         opt.defaultValue = QVariant(sdefval);
                         break;
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

} // namespace studio
} // namespace gams
