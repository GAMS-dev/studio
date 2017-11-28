#include "commandlinetokenizer.h"

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

} // namespace studio
} // namespace gams
