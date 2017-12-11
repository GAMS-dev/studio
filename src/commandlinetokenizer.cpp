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
    while(commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
        offset++;
    }
    QList<OptionItem> commandLineList;

    if (!commandLineStr.isEmpty()) {
        QString str  = commandLineStr.mid(offset);
        QStringList paramList = str.split(QRegExp("\\s+"));

        QStringList::const_iterator it = paramList.cbegin();
        while(it != paramList.cend()) {
            QString param = *it;
            QString key;
            QString value;
            int kpos = -1;
            int vpos = -1;
            if (param.contains("=")) {  // eg. param starts with "a=" or "=c" or "gdx=x" or "gdx=x=x.gdx"
                QStringList entry = param.split(QRegExp("="));
                key = entry.at(0);
                kpos = offset;
                offset += key.size();
                if (entry.size() > 2) { // param starts with "a=="
                    ++offset;
                    vpos = offset;
                    value = commandLineStr.mid(offset++, 1);
                    while(!commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                        value += commandLineStr.mid(offset++, 1);
                    }
                } else { // param starts with "a="
                    ++offset;  // move over "="
                    if (entry.at(1).isEmpty()) { // param starts with "a= =" or "a= c"
                        while(commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                            offset++;
                        }
                        ++it;
                        if (it == paramList.cend()) {
                            commandLineList.append(OptionItem(key, value, kpos, vpos));
                            break;
                        }
                        value = *it;
                        vpos = offset;
                        offset += value.size();
                    } else { // param starts with a=c
                        vpos = offset;
                        value = entry.at(1);
                        offset += value.size();
                    }
                    while(!commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                        offset++;
                    }
               }
             } else { // eg. param starts with "a =" or "a x" or "a ==" or "a = x" or "a = ="
                while(commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                    offset++;
                }
                if (commandLineStr.midRef(offset).startsWith("=")) {
                    ++offset;
                }
                while(commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                    offset++;
                }
                key = param;
                kpos = offset;
                offset += key.size();
                ++it;
                if (it == paramList.cend()) {
                   commandLineList.append(OptionItem(key, value, kpos, vpos));
                   while(commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                        ++offset;
                  }
                   break;
                } else {
                    value = *it;
                    while(commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                        offset++;
                    }
                    if (commandLineStr.midRef(offset).startsWith("=")) {
                        ++offset;
                    }
                    while(commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                        offset++;
                    }
                    vpos=offset;
                    offset += value.size();

                    if (value.startsWith("=")) {  // e.g. ("=","=x","x=x.gdx","=x=x.gdx") in  ("gdx = x","gdx =x", "gdx = x=x.gdx", "gdx =x=x.gdx");
                       value = value.mid(1);
                       --offset;
                       if (value.isEmpty()) {
                           vpos=offset;
                           ++it;
                           if (it == paramList.cend()) {
                               commandLineList.append(OptionItem(key, value, kpos, vpos));
                               break;
                           }
                           while(commandLineStr.midRef(offset).startsWith(" ")  && (offset< commandLineStr.length())) {
                               ++offset;
                           }

                           value = *it;
                           offset += value.size();
                       }
                   }
                }
             }
             commandLineList.append(OptionItem(key, value, kpos, vpos));
             if (it != paramList.cend()) {
                ++it;
                while(commandLineStr.midRef(offset).startsWith(" ") && (offset< commandLineStr.length())) {
                     ++offset;
                }
             }
        }  // end while
    }
    return commandLineList;
}

QList<OptionError> CommandLineTokenizer::format(const QList<OptionItem> &items)
{
    QList<OptionError> optionErrorList;
    if (!gamsOption->available())
        return optionErrorList;

    for (OptionItem item : items) {
        if (item.key.startsWith("--")) // ignore "--" parameter
            continue;
        else if (item.key.startsWith("-/")) // ignore "-/" parameter
                continue;
        else if (item.key.startsWith("/-")) // ignore "/-" parameter
                continue;
        else if (item.key.startsWith("//")) // ignore "//" parameter
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
           optionErrorList.append(OptionError(fr, item.value + QString(" (Option keyword expected for value \"%1\")").arg(item.value)) );
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
            } else { // neither invalid nor deprecated

                QString keyStr = key;
                if (!gamsOption->isValid(key))
                    key = gamsOption->getSynonym(key);

                if (gamsOption->getValueList(key).size() > 0) {

                    bool foundError = true;
                    int n = -1;
                    bool isCorrectDataType = false;
                    switch (gamsOption->getOptionType(key)) {
                    case optTypeEnumInt :
                       n = item.value.toInt(&isCorrectDataType);
                       if (isCorrectDataType) {
                         for (OptionValue optValue: gamsOption->getValueList(key)) {
                            if (optValue.value.toInt() == n) { // && !optValue.hidden) {
                                foundError = false;
                                break;
                            }
                         }
                       }
                       break;
                    case optTypeEnumStr :
                       for (OptionValue optValue: gamsOption->getValueList(key)) {
                           if (QString::compare(optValue.value.toString(), item.value, Qt::CaseInsensitive)==0) { //&& !optValue.hidden) {
                               foundError = false;
                               break;
                           }
                       }
                       break;
                    default:
                       foundError = false;  // do nothing for the moment
                       break;
                    }
                    if (foundError) {
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
                } else { // not enum

                    bool foundError = false;
                    bool isCorrectDataType = false;
                    QString errorMessage = item.value + " (value error ";
                    switch(gamsOption->getOptionType(key)) {
                     case optTypeInteger:
                        int n;
                         n = item.value.toInt(&isCorrectDataType);
                         if (isCorrectDataType) {
                            if ((n < gamsOption->getLowerBound(key).toInt()) ||
                                (gamsOption->getUpperBound(key).toInt() < n)) {
                                errorMessage.append( QString("for option \"%1\"), not in range [%2,%3]").arg(keyStr).arg(gamsOption->getLowerBound(key).toInt()).arg(gamsOption->getUpperBound(key).toInt()) );
                                foundError = true;
                            }
                         } else {
                             errorMessage.append( QString("for option \"%1\"), Integer expected").arg(keyStr) );
                             foundError = true;
                         }
                         break;
                     case optTypeDouble:
                         double d;
                         d = item.value.toDouble(&isCorrectDataType);
                         if (isCorrectDataType) {
                            if ((d < gamsOption->getLowerBound(key).toDouble()) ||
                                (gamsOption->getUpperBound(key).toDouble() < d)) {
                                errorMessage.append( QString("for option \"%1\"), not in range [%2,%3]").arg(keyStr).arg(gamsOption->getLowerBound(key).toDouble()).arg(gamsOption->getUpperBound(key).toDouble()) );
                                foundError = true;
                            }
                         } else {
                             errorMessage.append( QString("for option \"%1\"), Double expected").arg(keyStr) );
                             foundError = true;
                         }
                         break;
                     default:
                         foundError = false;  // do nothing for the moment
                         break;
                     }

                    if (foundError) {
                       QTextLayout::FormatRange fr;
                       fr.start = item.valuePosition;
                       fr.length = item.value.length();
                       fr.format = mInvalidValueFormat;
                       optionErrorList.append(OptionError(fr, errorMessage));
                    }
                 }
              }
        } // if (key.isEmpty()) { } else {
    } // for (OptionItem item : items)
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
