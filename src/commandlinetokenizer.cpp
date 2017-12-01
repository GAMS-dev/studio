#include "commandlinetokenizer.h"
#include "gamspaths.h"

namespace gams {
namespace studio {

CommandLineTokenizer::CommandLineTokenizer()
{
    gamsOption = new Option(GAMSPaths::systemDir(), QString("optgams.def"));

//    gamsOption->dumpAll();

    mInvalidKeyFormat.setFontItalic(true);
    mInvalidKeyFormat.setBackground(Qt::lightGray);
    mInvalidKeyFormat.setForeground(Qt::red);

    mInvalidValueFormat.setFontItalic(true);
    mInvalidValueFormat.setBackground(Qt::lightGray);
    mInvalidValueFormat.setForeground(Qt::blue);

    mDeprecateOptionFormat.setFontItalic(true);
    mDeprecateOptionFormat.setBackground(Qt::lightGray);
    mDeprecateOptionFormat.setForeground(Qt::white);
}

CommandLineTokenizer::~CommandLineTokenizer()
{
    delete gamsOption;
}

QList<OptionItem> CommandLineTokenizer::tokenize(const QString &commandLineStr)
{
    int offset = 0;
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
            if (param.contains("=")) {  // eg. param starts with "a=" or "=c" or "gdx=x" or "gdx=x=x.gdx"
                QStringList entry = param.split(QRegExp("="));
//                for (int i=0; i<entry.size(); ++i)
//                   qDebug() << QString(" entry %1 => %2").arg(i).arg(entry.at(i));
                key = entry.at(0);
                kpos = offset;
                offset += key.size();

                if (entry.size() > 2) { // param starts with "a=="
                    ++offset;
                    vpos = offset;
                    value = commandLineStr.mid(offset++, 1);
//                    qDebug() << QString("  checkpoint_0 value  => %1").arg(value);
                    while(!commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                        value += commandLineStr.mid(offset++, 1);
//                        qDebug() << QString(" checkpoint_0 value  => %1").arg(value);
                    }
                } else { // param starts with "a="
//                    qDebug() << QString("         [%1,%2] checkpoint_1").arg(key).arg(value);
                    ++offset;  // move over "="
                    if (entry.at(1).isEmpty()) { // param starts with "a= =" or "a= c"
                        while( commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                            offset++;
//                            qDebug() << QString(" checkpoint_1_1 offset  => %1").arg(offset);
                        }
                        ++it;
                        if (it == paramList.cend()) {
                            commandLineList.append(OptionItem(key, value, kpos, vpos));
//                            qDebug() << QString("         [%1,%2] checkpoint_2").arg(key).arg(value);
                            break;
                        }
                        value = *it;
                        vpos = offset;
                        offset += value.size();
//                        qDebug() << QString("         [%1,%2, %3] checkpoint_2_3").arg(key).arg(value).arg(offset);
                    } else { // param starts with a=c
                        vpos = offset;
                        value = entry.at(1);
                        offset += value.size();
//                        qDebug() << QString("         [%1,%2] entry.size()=%3 checkpoint_3").arg(key).arg(value).arg(entry.size());
                    }
                    while(!commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                        offset++;
//                        qDebug() << QString(" value  => [%1] checkpoint_xxx ").arg(offset);
                    }
               }
             } else { // eg. param starts with "a =" or "a x" or "a ==" or "a = x" or "a = ="
                key =  param;
                kpos = offset;
                offset += key.size();
                while(commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                    offset++;
                }
                if (commandLineStr.midRef(offset).startsWith("=")) {
                    ++offset;
                }
                while(commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                    offset++;
                }
//                qDebug() << QString("         [%1,%2,%3]  checkpoint_5_2").arg(key).arg(value).arg(offset);
                ++it;
                if (it == paramList.cend()) {
                   commandLineList.append(OptionItem(key, value, kpos, vpos));
                   break;
                } else {
                    value = *it;
//                    qDebug() << QString("         [%1,%2,%3] checkpoint_6").arg(key).arg(value).arg(offset);
                    vpos=offset;
                    offset += value.size();

                    if (value.startsWith("=")) {  // e.g. ("=","=x","x=x.gdx","=x=x.gdx") in  ("gdx = x","gdx =x", "gdx = x=x.gdx", "gdx =x=x.gdx");
                       value = value.mid(1);
                       --offset;
                       if (value.isEmpty()) {
//                           qDebug() << QString("         [%1,%2,%3] checkpoint_7").arg(key).arg(value).arg(offset);
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
//                           qDebug() << QString("         [%1,%2,%3] checkpoint_8").arg(key).arg(value).arg(offset);
                       }
                   }
                }
//                qDebug() << QString("         [%1,%2] checkpoint_9").arg(key).arg(value);
             }
             commandLineList.append(OptionItem(key, value, kpos, vpos));
             if (it != paramList.cend()) {
                ++it;  offset++;
             }
        }  // end while
    }
    return commandLineList;
}

QList<OptionError> CommandLineTokenizer::format(const QList<OptionItem> &items)
{
    QList<OptionError> optionErrorList;
    for (OptionItem item : items) {
        if (item.key.startsWith("--"))
            continue;

        QString key = item.key;
        if (key.startsWith("-"))
            key = key.mid(1);
        else if (key.startsWith("/"))
                key = key.mid(1);

        if (key.isEmpty()) {
           QTextLayout::FormatRange fr;
           fr.start = item.valuePosition;
           fr.length = item.value.size();
           fr.format = mInvalidValueFormat;
           optionErrorList.append(OptionError(fr, item.value + " (Option keyword expected)"));
        } else {
            if (!gamsOption->isValid(key) &&
                !gamsOption->isValid(gamsOption->getSynonym(key))
               ) {
                QTextLayout::FormatRange fr;
                fr.start = item.keyPosition;
                fr.length = item.key.length();
                fr.format = mInvalidKeyFormat;
                optionErrorList.append(OptionError(fr, key + " (Unknown option)"));
            } else if (gamsOption->isDeprecated(key)) {
                QTextLayout::FormatRange fr;
                fr.start = item.keyPosition;
                if (item.value.isEmpty())
                    fr.length = item.key.length();
                else
                   fr.length = (item.valuePosition + item.value.length()) - item.keyPosition;
                fr.format = mDeprecateOptionFormat;
                optionErrorList.append(OptionError(fr, key + " (Deprecated option, will be ignored)"));
            } else {
                QString keyStr = key;
                if (!gamsOption->isValid(key))
                    key = gamsOption->getSynonym(key);
                bool found = false;
//                qDebug() << QString("format -> comparing [%1] type [%2] [%3]").arg(key).arg(gamsOption->getType(key))
//                            .arg(gamsOption->getOptionTypeName(gamsOption->getType(key)));
                switch (gamsOption->getType(key)) {
                case optTypeEnumInt :
                    for (OptionValue optValue: gamsOption->getValueList(key)) {
//                        qDebug() << QString("  [%1, %2]").arg(optValue.value.toInt()).arg(item.value.toInt());
                        if (optValue.value.toInt()==item.value.toInt() && !optValue.hidden) {
                            found = true;
                            break;
                        }
                    }
                    break;
                case optTypeEnumStr :
                    for (OptionValue optValue: gamsOption->getValueList(key)) {
                        if (QString::compare(optValue.value.toString(), item.value, Qt::CaseInsensitive)==0
                            && !optValue.hidden) {
                            found = true;
                            break;
                        }
                    }
                    break;
                default:
                    found = true;  // do nothing for the moment
                    break;
                }
                if (!found) {
                    QTextLayout::FormatRange fr;
                    fr.start = item.valuePosition;
                    fr.length = item.value.length();
                    fr.format = mInvalidValueFormat;
                    QString errorMessage = item.value + " (unknown value for option \""+keyStr+"\")";
                    if (gamsOption->getValueList(key).size() > 0) {
                        errorMessage += ", Possible values are ";
                        for (OptionValue optValue: gamsOption->getValueList(key)) {
                            if (optValue.hidden)
                                continue;
                            errorMessage += optValue.value.toString();
                            errorMessage += " ";
                        }
                    }
                    optionErrorList.append(OptionError(fr, errorMessage));
                }
            }
        }
    }
    return optionErrorList;
}

QTextCharFormat CommandLineTokenizer::invalidKeyFormat() const
{
    return mInvalidKeyFormat;
}

QTextCharFormat CommandLineTokenizer::invalidValueFormat() const
{
    return mInvalidValueFormat;
}

QTextCharFormat CommandLineTokenizer::deprecateOptionFormat() const
{
    return mDeprecateOptionFormat;
}

} // namespace studio
} // namespace gams
