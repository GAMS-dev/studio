#include "commandlinetokenizer.h"
#include "gamspaths.h"

namespace gams {
namespace studio {

CommandLineTokenizer::CommandLineTokenizer()
{
    gamsOption = new Option(GAMSPaths::systemDir(), QString("optgams.def"));

    gamsOption->dumpAll();

    mInvalidKeyFormat.setFontItalic(true);
    mInvalidKeyFormat.setBackground(Qt::lightGray);
    mInvalidKeyFormat.setForeground(Qt::blue);

    mInvalidValueFormat.setFontItalic(true);
    mInvalidValueFormat.setBackground(Qt::lightGray);
    mInvalidValueFormat.setForeground(Qt::red);

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

QList<OptionError> CommandLineTokenizer::format(const QList<OptionItem> &items)
{
    QList<OptionError> optionErrorList;
    for (OptionItem item : items) {
        if (item.key.isEmpty()) {
           QTextLayout::FormatRange fr;
           fr.start = item.valuePosition;
           fr.length = item.value.size();
           fr.format = mInvalidValueFormat;
           optionErrorList.append(OptionError(fr, NoKey));
        } else {
            if (!gamsOption->isValid(item.key) &&
                !gamsOption->isValid(gamsOption->getSynonym(item.key))
               ) {
                QTextLayout::FormatRange fr;
                fr.start = item.keyPosition;
                fr.length = item.key.length();
                fr.format = mInvalidKeyFormat;
                optionErrorList.append(OptionError(fr, InvalidKey));
            } else if (gamsOption->isDeprecated(item.key)) {
                QTextLayout::FormatRange fr;
                fr.start = item.keyPosition;
                if (item.value.isEmpty())
                    fr.length = item.key.length();
                else
                   fr.length = (item.valuePosition + item.value.length()) - item.keyPosition;
                fr.format = mDeprecateOptionFormat;
                optionErrorList.append(OptionError(fr, Deprecated));
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
