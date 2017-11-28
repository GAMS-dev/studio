#include "commandlinetokenizer.h"

namespace gams {
namespace studio {

CommandLineTokenizer::CommandLineTokenizer()
{
    mInvalidKeyFormat.setFontItalic(true);
    mInvalidKeyFormat.setBackground(Qt::darkYellow);
    mInvalidKeyFormat.setForeground(Qt::white);

    mInvalidValueFormat.setFontItalic(true);
    mInvalidKeyFormat.setBackground(Qt::darkYellow);
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
                offset++;
                value = entry.at(1);
                vpos = offset;   offset = offset+value.size();
//                qDebug() << QString("         [%1,%2] checkpoint_1").arg(key).arg(value);
                if (value.isEmpty()) {
                    ++it;
                    if (it == paramList.cend()) {
                        commandLineList.append(OptionItem(key, value, kpos, vpos));
//                        qDebug() << QString("         [%1,%2] checkpoint_2").arg(key).arg(value);
                        break;
                    }
                    value = *it;
                    offset += value.size();
//                    qDebug() << QString("         [%1,%2] checkpoint_3").arg(key).arg(value);
                } else {
                    int i = 2;
                    while(i < entry.size()) {
                        QString str = entry.at(i++);
                        value.append("=").append(str);
                        offset = offset + 1;
                        offset = offset + str.size();
                    }
//                    qDebug() << QString("         [%1,%2] entry.size()=%3 checkpoint_3").arg(key).arg(value).arg(entry.size());
                }
            } else {
               key =  param;
               kpos = offset;
               offset += key.size();
               offset++;
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
                       if (value.isEmpty()) {
//                           qDebug() << QString("         [%1,%2] checkpoint_7").arg(key).arg(value);
                           offset++;
                           vpos=offset;
                           ++it;
                           if (it == paramList.cend()) {
                               commandLineList.append(OptionItem(key, value, kpos, vpos));
                               break;
                           }
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
           qDebug() << QString("key(%1=%2) empty %3 %4").arg(item.key).arg(item.value).arg(fr.start).arg(fr.length);
        } else if (item.value.isEmpty()) {
            QTextLayout::FormatRange fr;
            fr.start = item.key.size();
            fr.length = item.key.length();
            fr.format = mInvalidKeyFormat;
            frList.append(fr);
            qDebug() << QString("value(%1=%2) empty %3 %4").arg(item.key).arg(item.value).arg(fr.start).arg(fr.length);;
        } else {
           qDebug()  << QString("value(%1=%2)").arg(item.key).arg(item.value);
        }
    }
    // TODO
    return frList;
}

} // namespace studio
} // namespace gams
