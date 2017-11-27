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
    QString parsedStr = commandLineStr;
    unsigned int offset = 0;
    QList<OptionItem> commandLineList;

    if (!commandLineStr.isEmpty()) {
        QStringList paramList = commandLineStr.split(QRegExp("\\s+"));

        QStringList::iterator it = paramList.begin();
        while(it != paramList.end()) {
            QString param = *it;
            QString key;
            QString value;
            int kpos = -1;
            int vpos = -1;
            if (param.contains("=")) {  // eg. "=c" or "gdx=x" or "gdx=x=x.gdx"
                QStringList entry = param.split(QRegExp("="));
                key = entry.at(0);
                kpos = offset;
                offset += key.size();   parsedStr = commandLineStr.mid(offset);
                offset++;                parsedStr = commandLineStr.mid(offset);   // "="
                value = entry.at(1);
                vpos = offset;   offset = offset + value.size();  parsedStr = commandLineStr.mid(offset);
                if (value.isEmpty()) {
                    ++it;
                    value = *it;
                    offset += value.size();     parsedStr = commandLineStr.mid(offset);
                } else {
                    int i = 2;
                    while(i < entry.size()) {
                        QString str = entry.at(i++);
                        value.append("=").append(str);
                        offset = offset + 1;
                        offset = offset + str.size();  parsedStr = commandLineStr.mid(offset);
                    }
                }
            } else {
               key =  param;
               kpos = offset;
               offset += key.size();  parsedStr = commandLineStr.mid(offset);
               offset++;
               if (++it != paramList.end()) {
                   value = *it;
                   vpos=offset;
                   offset += value.size();   parsedStr = commandLineStr.mid(offset);

                   if (value.startsWith("=")) {  // e.g. ("=","=x","x=x.gdx","=x=x.gdx") in  ("gdx = x","gdx =x", "gdx = x=x.gdx", "gdx =x=x.gdx");
                       QString  str = value.mid(1);
                       if (str.isEmpty()) {
                           offset++;
                           vpos=offset;
                           ++it;
                           value = *it;
                           offset += value.size();  parsedStr = commandLineStr.mid(offset);
                       }
                   }
               }
            }
            commandLineList.append(OptionItem(key, value, kpos, vpos));
            ++it;  offset++;
        }

    }
    return commandLineList;
}

QList<QTextLayout::FormatRange> CommandLineTokenizer::format(const QList<OptionItem> &item)
{

}

} // namespace studio
} // namespace gams
