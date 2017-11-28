#include "commandlinetokenizer.h"
#include "exception.h"
#include "gclgms.h"

namespace gams {
namespace studio {

CommandLineTokenizer::CommandLineTokenizer()
{
    mInvalidKeyFormat.setFontItalic(true);
    mInvalidKeyFormat.setBackground(Qt::lightGray);
    mInvalidKeyFormat.setForeground(Qt::blue);

    mInvalidValueFormat.setFontItalic(true);
    mInvalidValueFormat.setBackground(Qt::lightGray); //Qt::darkYellow);
    mInvalidValueFormat.setForeground(Qt::red);
}

QList<OptionItem> CommandLineTokenizer::tokenize(const QString &commandLineStr)
{
    unsigned int offset = 0;
    QList<OptionItem> commandLineList;
//    qDebug() << QString("tokenize => %1").arg(commandLineStr);

    if (!commandLineStr.isEmpty()) {
        QStringList paramList = commandLineStr.split(QRegExp("\\s+"));

        QStringList::const_iterator it = paramList.cbegin();
        while(it != paramList.cend()) {
            QString param = *it;
//            qDebug() << QString("           param=[%1]").arg(param);
            QString key;
            QString value;
            int kpos = -1;
            int vpos = -1;
            if (param.contains("=")) {  // eg. "a=" or "=c" or "gdx=x" or "gdx=x=x.gdx"
                QStringList entry = param.split(QRegExp("="));
                key = entry.at(0);
                kpos = offset;
                offset += key.size();
                while(commandLineStr.midRef(offset).startsWith("=") || commandLineStr.midRef(offset).startsWith(" "))
                    ++offset;
                value = entry.at(1);
                vpos = offset;
                offset = offset+value.size();
//                qDebug() << QString("         [%1,%2] checkpoint_1").arg(key).arg(value);
                if (value.isEmpty()) {
                    ++it;
                    if (it == paramList.cend()) {
                        commandLineList.append(OptionItem(key, value, kpos, vpos));
//                        qDebug() << QString("         [%1,%2] checkpoint_2").arg(key).arg(value);
                        break;
                    }
                    value = *it;
                    while(commandLineStr.midRef(offset).startsWith("=") || commandLineStr.midRef(offset).startsWith(" "))
                        ++offset;
                    offset += value.size();
//                    qDebug() << QString("         [%1,%2] checkpoint_3").arg(key).arg(value);
                } else {
                    int i = 2;
                    while(i < entry.size()) {
                        QString str = entry.at(i++);
                        value.append("=").append(str);
                        offset = offset + 1 + str.size();
                    }
//                    qDebug() << QString("         [%1,%2] entry.size()=%3 checkpoint_3").arg(key).arg(value).arg(entry.size());
                }
            } else {
               key =  param;
               kpos = offset;
               offset += key.size();
               while(commandLineStr.midRef(offset).startsWith("=") || commandLineStr.midRef(offset).startsWith(" "))
                   ++offset;
//               qDebug() << QString("         [%1,%2] checkpoint_5").arg(key).arg(value);
               ++it;
               if (it == paramList.cend()) {
                   commandLineList.append(OptionItem(key, value, kpos, vpos));
                   break;
               } else {
                   value = *it;
//                   qDebug() << QString("         [%1,%2] checkpoint_6").arg(key).arg(value);
                   vpos=offset;
                   offset += value.size();

                   if (value.startsWith("=")) {  // e.g. ("=","=x","x=x.gdx","=x=x.gdx") in  ("gdx = x","gdx =x", "gdx = x=x.gdx", "gdx =x=x.gdx");
                       value = value.mid(1);
                       --offset;
                       if (value.isEmpty()) {
                           qDebug() << QString("         [%1,%2] checkpoint_7").arg(key).arg(value);
                           vpos=offset;
                           ++it;
                           if (it == paramList.cend()) {
                               commandLineList.append(OptionItem(key, value, kpos, vpos));
                               break;
                           }
                           while(commandLineStr.midRef(offset).startsWith(" "))
                               ++offset;

                           value = *it;
                           offset += value.size();
//                           qDebug() << QString("         [%1,%2] checkpoint_8").arg(key).arg(value);
                       }
                   }
               }
//               qDebug() << QString("         [%1,%2] checkpoint_9").arg(key).arg(value);
            }
            commandLineList.append(OptionItem(key, value, kpos, vpos));
            if (it != paramList.cend()) {
                ++it;  offset++;
            }
        }

    }
    return commandLineList;
}

QList<QTextLayout::FormatRange> CommandLineTokenizer::format(const QList<OptionItem> &items)
{
    QList<QTextLayout::FormatRange> frList;
    for (OptionItem item : items) {
        if (item.key.isEmpty()) {
           QTextLayout::FormatRange fr;
           fr.start = item.valuePosition;
           fr.length = item.value.size();
           fr.format = mInvalidValueFormat;
           frList.append(fr);
        } else if (item.value.isEmpty()) {
            QTextLayout::FormatRange fr;
            fr.start = item.keyPosition;
            fr.length = item.key.length();
            fr.format = mInvalidKeyFormat;
            frList.append(fr);
        }
    }
    return frList;
}

void CommandLineTokenizer::readDefinition(QString systemPath, QString optionFileName)
{
    optHandle_t mOPTHandle;

    char msg[GMS_SSSIZE];
//    const char* sysPath = QDir::toNativeSeparators(systemPath).toLatin1().data();
    qDebug() << QString(systemPath.toLatin1());
    optCreateD(&mOPTHandle, systemPath.toLatin1(), msg, sizeof(msg));
    if (msg[0] != '\0')
        EXCEPT() << "ERROR" << msg;

    qDebug() << QDir(systemPath).filePath("optgams.def").toLatin1();


    if (!optReadDefinition(mOPTHandle, QDir(systemPath).filePath("optgams.def").toLatin1())) {

        qDebug() << "optCount:" << optCount(mOPTHandle)
                 << ", optMessageCount:" << optMessageCount(mOPTHandle)
                 << ", optGroupCount:" << optGroupCount(mOPTHandle)
                 << ", optSynonymCount:" << optSynonymCount(mOPTHandle)
                 << ", optRecentEnabled:" << optRecentEnabled(mOPTHandle);

         QMap<QString, QString> synonym;
         char name[GMS_SSSIZE];
         char syn[GMS_SSSIZE];
         for (int i = 1; i <= optSynonymCount(mOPTHandle); ++i) {
             optGetSynonym(mOPTHandle, i, syn, name);
             synonym[name] = syn;
         }

         for (int i=1; i <= optGroupCount(mOPTHandle); ++i) {
             char name[GMS_SSSIZE];
             char help[GMS_SSSIZE];
             int helpContextNr;
             int group;
             optGetGroupNr(mOPTHandle, i, name, &group, &helpContextNr, help);
//             qDebug() << QString("%1: %2 group_%3 %4 help_%5").arg(i).arg(name).arg(group).arg(helpContextNr).arg(help);
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
//                     char sname[GMS_SSSIZE];
//                     char sval[GMS_SSSIZE];

                     optGetHelpNr(mOPTHandle, i, name, descript);
                     optGetInfoNr(mOPTHandle, i, &idefined, &idummy, &irefnr, &itype, &iopttype, &ioptsubtype);

                     OptionDefinition opt(QString::fromLatin1(name), static_cast<optOptionType>(iopttype), static_cast<optDataType>(itype), QString::fromLatin1(descript));

                     int helpContextNr;
                     optGetOptHelpNr(mOPTHandle, i, name, &helpContextNr, &group);
                     opt.groupNumber = group;

                     if (synonym.contains(name)) {
                         opt.synonym = synonym[name];
                         mSynonymMap[synonym[name]] = name;
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


                     int count = optListCountStr(mOPTHandle, name);
                     optGetEnumCount(mOPTHandle, i, &enumCount);
//                     qDebug() << QString("%1 = ").arg(name) << QString::number(enumCount) << QString(", subtype=%1, %2").arg(ioptsubtype).arg(count);
//                     for (int c = 1; c<= count ; ++i) {
//                         char msg[GMS_SSSIZE];
//                         optReadFromListStr(mOPTHandle, name, c, msg);
//                         qDebug() << QString("   %1 msg=%2").arg(c).arg(msg);
//                     }

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

                     mOption.append( opt );
         }
     } else {
          EXCEPT() << "Problem reading definition file " << QDir(systemPath).filePath("optgams.def").toLatin1();
     }

     optFree(&mOPTHandle);
}

void CommandLineTokenizer::dumpOptionDefinition()
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

    qDebug() << QString("mOption.size() = %1").arg(mOption.size());
    for (int i = 0; i < mOption.size(); ++i) {
                OptionDefinition opt = mOption.at(i);
                qDebug() << QString(" %1: %2 [%3] type_%4 %5 range_[%6,%7] group_%8").arg(i).arg(opt.name).arg(opt.synonym).arg(mOptionTypeNameMap[opt.type]).arg(opt.description)
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

} // namespace studio
} // namespace gams
