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
#include <QCoreApplication>
#include <QDir>
#include <QTextStream>

#include <QDebug>

#include "optiontokenizer.h"
#include "gclgms.h"
#include "option.h"
#include "commonpaths.h"
#include "locators/defaultsystemlogger.h"
#include "locators/sysloglocator.h"

namespace gams {
namespace studio {
namespace option {

AbstractSystemLogger* OptionTokenizer::mNullLogger = new DefaultSystemLogger;

OptionTokenizer::OptionTokenizer(const QString &optionFileName)
{
    mOption = new Option(CommonPaths::systemDir(), optionFileName);

    mInvalidKeyFormat.setFontItalic(true);
    mInvalidKeyFormat.setBackground(Qt::lightGray);
    mInvalidKeyFormat.setForeground(Qt::red);

    mInvalidValueFormat.setFontItalic(true);
    mInvalidValueFormat.setBackground(Qt::lightGray);
    mInvalidValueFormat.setForeground(Qt::red/*Qt::blue*/);

    mDeprecateOptionFormat.setFontItalic(true);
    mDeprecateOptionFormat.setBackground(Qt::lightGray);
    mDeprecateOptionFormat.setForeground(Qt::white);

    mDeactivatedOptionFormat.setFontItalic(true);
    mDeactivatedOptionFormat.setForeground(Qt::lightGray);
}

OptionTokenizer::~OptionTokenizer()
{
    delete mOption;
}

QList<OptionItem> OptionTokenizer::tokenize(const QString &commandLineStr)
{
    QList<OptionItem> commandLineList;
    if (!commandLineStr.isEmpty()) {

        int offset = 0;
        int length = commandLineStr.length();
        QStringRef str = commandLineStr.midRef(0);
        offsetWhiteSpaces(str, offset, length);
        while( offset < commandLineStr.length() ) {
            QString key = "";
            QString value = "";
            int keyPosition = -1;
            int valuePosition = -1;

            offsetKey(str, key, keyPosition, offset, length);
            if (offset >= commandLineStr.length()) {
                commandLineList.append(OptionItem(key, value, keyPosition, valuePosition));
                break;
            }

            offsetAssignment(str, offset, length);
            if (offset >= commandLineStr.length()) {
                commandLineList.append(OptionItem(key, value, keyPosition, valuePosition));
                break;
            }

            offsetValue(str, value, valuePosition, offset, length);

            commandLineList.append(OptionItem(key, value, keyPosition, valuePosition));

            offsetWhiteSpaces(str, offset, length);
            if (offset >= commandLineStr.length()) {
                break;
            }
        }
    }

    return commandLineList;
}

QList<OptionItem> OptionTokenizer::tokenize(const QString &commandLineStr, const QList<QString> &disabledOption)
{
    QList<OptionItem> commandLineList;
    if (!commandLineStr.isEmpty()) {
        int offset = 0;
        int length = commandLineStr.length();
        QStringRef str = commandLineStr.midRef(0);
        offsetWhiteSpaces(str, offset, length);
        while( offset < commandLineStr.length() ) {
            QString key = "";
            QString value = "";
            int keyPosition = -1;
            int valuePosition = -1;

            offsetKey(str, key, keyPosition, offset, length);
            bool disabled = (disabledOption.contains(key));
            if (offset >= commandLineStr.length()) {
                commandLineList.append(OptionItem(key, value, keyPosition, valuePosition, disabled));
                break;
            }

            offsetAssignment(str, offset, length);
            if (offset >= commandLineStr.length()) {
                commandLineList.append(OptionItem(key, value, keyPosition, valuePosition, disabled));
                break;
            }

            offsetValue(str, value, valuePosition, offset, length);

            commandLineList.append(OptionItem(key, value, keyPosition, valuePosition, disabled));

            offsetWhiteSpaces(str, offset, length);
            if (offset >= commandLineStr.length()) {
                break;
            }
        }
    }
    return commandLineList;
}

QList<OptionError> OptionTokenizer::format(const QList<OptionItem> &items)
{
    QList<OptionError> optionErrorList;
    if (!mOption->available())
        return optionErrorList;

    for (OptionItem item : items) {
        if (item.disabled) {
            QTextLayout::FormatRange fr;
            fr.start = item.keyPosition;
            if (item.value.isEmpty())
                fr.length = item.key.length()+1;  // also format '='  after the key
            else
               fr.length = (item.valuePosition + item.value.length()) - item.keyPosition;
            fr.format = mDeactivatedOptionFormat;
            optionErrorList.append(OptionError(fr, "")); //item.key + QString(" (Option will be disabled in the next run)")) );
            continue;
        }
        if (mOption->isDoubleDashedOption(item.key)) { //( item.key.startsWith("--") || item.key.startsWith("-/") || item.key.startsWith("/-") || item.key.startsWith("//") ) { // double dash parameter
            QString optionKey = mOption->getOptionKey(item.key);
            if (!mOption->isDoubleDashedOptionNameValid( optionKey ))   {
                QTextLayout::FormatRange fr;
                fr.start = item.keyPosition;
                fr.length = item.key.length();
                fr.format = mInvalidKeyFormat;
                optionErrorList.append(OptionError(fr, optionKey + QString(" (Either start with other character than [a-z or A-Z], or a subsequent character is not one of (a-z, A-Z, 0-9, or _))") ) );
            }
            continue;
        }

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

            if (!mOption->isValid(key) && (!mOption->isASynonym(key)) // &&!gamsOption->isValid(gamsOption->getSynonym(key))
               ) {
                QTextLayout::FormatRange fr;
                fr.start = item.keyPosition;
                fr.length = item.key.length();
                fr.format = mInvalidKeyFormat;
                optionErrorList.append(OptionError(fr, key + " (Unknown option)"));
            } else if (mOption->isDeprecated(key)) {
                QTextLayout::FormatRange fr;
                fr.start = item.keyPosition;
                if (item.value.isEmpty())
                    fr.length = item.key.length();
                else
                   fr.length = (item.valuePosition + item.value.length()) - item.keyPosition;
                fr.format = mDeprecateOptionFormat;

                switch (mOption->getValueErrorType(key, item.value)) {
                case Incorrect_Value_Type:
                case Value_Out_Of_Range:
                    optionErrorList.append(OptionError(fr, item.value + QString(" (Invalid value for deprecated option \"%1\", option will be eventually ignored)").arg(key)) );
                    break;
                case No_Error:
                default:
                    optionErrorList.append(OptionError(fr, key + " (Deprecated option, will be ignored)"));
                    break;
                }
            } else { // neither invalid nor deprecated key

                QString keyStr = key;
                if (!mOption->isValid(key))
                    key = mOption->getNameFromSynonym(key);

                QString value = item.value;

                if (item.value.startsWith("\"") && item.value.endsWith("\"")) { // peel off double quote
                    value = item.value.mid(1, item.value.length()-2);
                }
                if (value.contains("\"")) { // badly double quoted
                    QTextLayout::FormatRange fr;
                    fr.start = item.valuePosition;
                    fr.length = item.value.length();
                    fr.format = mInvalidValueFormat;
                    optionErrorList.append(OptionError(fr, QString("%1 (value error, bad double quoted value)").arg(item.value) ));
                    continue;
                }

                if (mOption->getValueList(key).size() > 0) { // enum type

                    bool foundError = true;
                    int n = -1;
                    bool isCorrectDataType = false;
                    switch (mOption->getOptionType(key)) {
                    case optTypeEnumInt :
                       n = value.toInt(&isCorrectDataType);
                       if (isCorrectDataType) {
                         for (OptionValue optValue: mOption->getValueList(key)) {
                            if (optValue.value.toInt() == n) { // && !optValue.hidden) {
                                foundError = false;
                                break;
                            }
                         }
                       }
                       break;
                    case optTypeEnumStr :
                       for (OptionValue optValue: mOption->getValueList(key)) {
                           if (QString::compare(optValue.value.toString(), value, Qt::CaseInsensitive)==0) { //&& !optValue.hidden) {
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
                       QString errorMessage = value + " (unknown value for option \""+keyStr+"\")";
                       if (mOption->getValueList(key).size() > 0) {
                          errorMessage += ", Possible values are ";
                          for (OptionValue optValue: mOption->getValueList(key)) {
                             if (optValue.hidden)
                                continue;
                             errorMessage += optValue.value.toString();
                             errorMessage += " ";
                          }
                       }
                       optionErrorList.append(OptionError(fr, errorMessage));
                   }
                } else { // not enum
                    switch(mOption->getValueErrorType(key, item.value)) {
                    case Value_Out_Of_Range: {
                        QString errorMessage = value + " (value error for option ";
                        errorMessage.append( QString("\"%1\"), not in range [%2,%3]").arg(keyStr).arg(mOption->getLowerBound(key).toDouble()).arg(mOption->getUpperBound(key).toDouble()) );
                        QTextLayout::FormatRange fr;
                        fr.start = item.valuePosition;
                        fr.length = item.value.length();
                        fr.format = mInvalidValueFormat;
                        optionErrorList.append(OptionError(fr, errorMessage));
                        break;
                    }
                    case Incorrect_Value_Type: {
                        bool foundError = false;
                        bool isCorrectDataType = false;
                        QString errorMessage = value + " (value error for option ";
                        if (mOption->getOptionType(key) == optTypeInteger) {
                            value.toInt(&isCorrectDataType);
                            if (!isCorrectDataType) {
                                errorMessage.append( QString("\"%1\"), Integer expected").arg(keyStr) );
                                foundError = true;
                            }
                        } else {
                            value.toDouble(&isCorrectDataType);
                            if (!isCorrectDataType) {
                                errorMessage.append( QString("\"%1\"), Double expected").arg(keyStr) );
                                foundError = true;
                            }
                        }
                        if (foundError) {
                            QTextLayout::FormatRange fr;
                            fr.start = item.valuePosition;
                            fr.length = item.value.length();
                            fr.format = mInvalidValueFormat;
                            optionErrorList.append(OptionError(fr, errorMessage));
                        }
                        break;
                    }
                    case No_Error:
                    default:
                        break;
                    }
                 }
              }
        } // if (key.isEmpty()) { } else {
    } // for (OptionItem item : items)
    return optionErrorList;
}

QString OptionTokenizer::normalize(const QString &commandLineStr)
{
    return normalize( tokenize(commandLineStr) );
}

QString OptionTokenizer::normalize(const QList<OptionItem> &items)
{
    QStringList strList;
    for (OptionItem item : items) {

        if ( item.key.isEmpty() )
            item.key = "[KEY]";
        if ( item.value.isEmpty() )
            item.value = "[VALUE]";

        if ( item.key.startsWith("--") || item.key.startsWith("-/") || item.key.startsWith("/-") || item.key.startsWith("//") ) { // double dash parameter
            strList.append(item.key+"="+item.value);
            continue;
        }
        QString key = item.key;
        if (key.startsWith("-") || key.startsWith("/"))
            key = key.mid(1);

        strList.append(key+"="+item.value);
    }
    return strList.join(" ");
}

void OptionTokenizer::offsetWhiteSpaces(QStringRef str, int &offset, const int length)
{
    while( str.mid(offset).startsWith(" ") && (offset < length) ) {
           ++offset;
    }
}

void OptionTokenizer::offsetKey(QStringRef str, QString &key, int &keyPosition, int &offset, const int length)
{
    if (keyPosition == -1)
       keyPosition = offset;
    while( offset < length ) {
        if  (str.mid(offset).startsWith(" ") || str.mid(offset).startsWith("="))
            break;
        key += str.mid(offset, 1);
        ++offset;
    }
}

void OptionTokenizer::offsetAssignment(QStringRef str, int &offset, const int length)
{
    bool seenAssignmentOperator = false;
    while( (offset < length) &&
           (str.mid(offset).startsWith(" ") || str.mid(offset).startsWith("="))
         )
    {
        if (str.mid(offset).startsWith("=")) {
            if (!seenAssignmentOperator)
               seenAssignmentOperator = true;
            else
                break;
        }
        ++offset;
    }
}

void OptionTokenizer::offsetValue(QStringRef str, QString &value, int &valuePosition, int &offset, const int length)
{
    bool startedWithDoubleQuote = false;
    bool seenCompleteDoubleQuotation = false;
    if (offset < length &&  str.mid(offset).startsWith("\"") ) {
        startedWithDoubleQuote = true;
        valuePosition = offset;
        value += str.mid(offset, 1);
        ++offset;
    }
    while( offset < length ) {

        if (!startedWithDoubleQuote) {
            if (str.mid(offset).startsWith(" ")) {
               break;
            }
        } else { // start with double quote
            if (seenCompleteDoubleQuotation && str.mid(offset).startsWith(" ")) {
                break;
            } else  { // seen only first double quote so far or currently not a whitespace
                if (str.mid(offset).startsWith("\"")) { // currently encounter a double quote
                    seenCompleteDoubleQuotation = true;
                }
            }
        }

        if (valuePosition == -1)
            valuePosition = offset;
        value += str.mid(offset, 1);
        ++offset;
    }
}

QTextCharFormat OptionTokenizer::invalidKeyFormat() const
{
    return mInvalidKeyFormat;
}

QTextCharFormat OptionTokenizer::invalidValueFormat() const
{
    return mInvalidValueFormat;
}

QTextCharFormat OptionTokenizer::deprecateOptionFormat() const
{
    return mDeprecateOptionFormat;
}

void OptionTokenizer::setInvalidKeyFormat(const QTextCharFormat &invalidKeyFormat)
{
    mInvalidKeyFormat = invalidKeyFormat;
}

void OptionTokenizer::setInvalidValueFormat(const QTextCharFormat &invalidValueFormat)
{
    mInvalidValueFormat = invalidValueFormat;
}

void OptionTokenizer::setDeprecateOptionFormat(const QTextCharFormat &deprecateOptionFormat)
{
    mDeprecateOptionFormat = deprecateOptionFormat;
}

void OptionTokenizer::setDeactivatedOptionFormat(const QTextCharFormat &deactivatedOptionFormat)
{
    mDeactivatedOptionFormat = deactivatedOptionFormat;
}

void OptionTokenizer::formatTextLineEdit(QLineEdit* lineEdit, const QString &commandLineStr)
{
//    this->setLineEditTextFormat(lineEdit, "");
    QList<OptionError> errList;
    if (!commandLineStr.isEmpty())
        errList = this->format( this->tokenize(commandLineStr) );

    this->formatLineEdit(lineEdit, errList);
}

void OptionTokenizer::formatItemLineEdit(QLineEdit* lineEdit, const QList<OptionItem> &optionItems)
{
    QString commandLineStr = this->normalize(optionItems);
    lineEdit->setText(commandLineStr );

    QList<OptionItem> tokenizedItems = this->tokenize(commandLineStr);
    for(int i=0; i<optionItems.size(); ++i) {
         tokenizedItems[i].disabled = optionItems[i].disabled;
    }
    QList<OptionError> errList = this->format( tokenizedItems );
    this->formatLineEdit(lineEdit, errList);
}

bool OptionTokenizer::logMessage(optHandle_t &mOPTHandle)
{
    bool hasbeenLogged = false;
    int itype;
    char svalue[GMS_SSSIZE];
    for (int i = 1; i <= optMessageCount(mOPTHandle); ++i) {
       optGetMessage(mOPTHandle, i, svalue, &itype );
       qDebug() << "#Message" << i << ":" << svalue << ":" << itype;
       if (itype==optMsgTooManyMsgs)
           continue;
       hasbeenLogged = true;
       switch (itype) {
       case optMsgHelp:
           logger()->appendLog(QString::fromLatin1(svalue), LogMsgType::Info);
           break;
       case optMsgValueWarning :
       case optMsgDeprecated :
           logger()->appendLog(QString::fromLatin1(svalue), LogMsgType::Warning);
           break;
       case optMsgDefineError:
       case optMsgValueError:
       case optMsgUserError:
           logger()->appendLog(QString::fromLatin1(svalue), LogMsgType::Error);
           break;
       default:
           break;
       }
    }
    optClearMessages(mOPTHandle);
    return hasbeenLogged;
}

QList<OptionItem> OptionTokenizer::readOptionParameterFile(const QString &absoluteFilePath)
{
    QList<OptionItem> items;

    optHandle_t mOPTHandle;

    char msg[GMS_SSSIZE];
    optCreateD(&mOPTHandle, mOption->getOptionDefinitionPath().toLatin1(), msg, sizeof(msg));
    if (msg[0] != '\0') {
        logger()->appendLog(msg, LogMsgType::Error);
        optFree(&mOPTHandle);
        return items;
    }

    mOption->resetModficationFlag();

    if (!optReadDefinition(mOPTHandle, QDir(mOption->getOptionDefinitionPath()).filePath(mOption->getOptionDefinitionFile()).toLatin1())) {
       if (!optReadParameterFile(mOPTHandle, absoluteFilePath.toLatin1())) {
           logMessage(mOPTHandle);
           optResetAllRecent(mOPTHandle);
           for (int i = 1; i <= optCount(mOPTHandle); ++i) {
               int idefined;
               int itype;
               int iopttype;
               int ioptsubtype;
               int idummy;
               int irefnr;
               optGetInfoNr(mOPTHandle, i, &idefined, &idummy, &irefnr, &itype, &iopttype, &ioptsubtype);

               if (iopttype == optTypeImmediate)
                  continue;

               if (idefined==0)  // no modification
                   continue;

               char name[GMS_SSSIZE];
               int group = 0;
               int helpContextNr;
               int ivalue;
               double dvalue;
               char svalue[GMS_SSSIZE];

               optGetOptHelpNr(mOPTHandle, i, name, &helpContextNr, &group);
               optGetValuesNr(mOPTHandle, i, name, &ivalue, &dvalue, svalue);

               mOption->setModified(QString::fromLatin1(name), true);
               QVariant value;
               switch(itype) {
               case optDataInteger: {
                   qDebug() << QString("%1:%2=[%3](int)").arg(i).arg(name).arg(ivalue);
                   value = QVariant(ivalue);
                   items.append(OptionItem(name, value.toString()));
                   break;
               }
               case optDataDouble: {
                   qDebug() << QString("%1:%2=[%3](double)").arg(i).arg(name).arg(dvalue);
                   value = QVariant(dvalue);
                   items.append(OptionItem(name, value.toString()));
                   break;
               }
               case optDataString: {
                   qDebug() << QString("%1:%2=[%3](string)").arg(i).arg(name).arg(svalue);
                   value = QVariant(svalue);
                   items.append(OptionItem(name, value.toString()));
                   break;
               }
               case optDataStrList: {
                   qDebug() << QString("%1:%2=[%3](stringlist)").arg(i).arg(name).arg(svalue);

                   for (int j = 1; j <= optListCountStr(mOPTHandle, name ); ++j) {
                      optReadFromListStr( mOPTHandle, name, j, svalue );
                      qDebug() << QString(" %1").arg(svalue);
                   }
                   qDebug() << QString("--> %1").arg(svalue);
                   value = QVariant(svalue);
                   items.append(OptionItem(name, value.toString()));
                   break;
               }
               case optDataNone:
               default:
                   break;
               }
            }
       }
    }
    logMessage(mOPTHandle);
    optFree(&mOPTHandle);

    return items;
}

bool OptionTokenizer::writeOptionParameterFile(const QList<OptionItem> &items, const QString &absoluteFilepath)
{
    optHandle_t mOPTHandle;

    char msg[GMS_SSSIZE];
    optCreateD(&mOPTHandle, mOption->getOptionDefinitionPath().toLatin1(), msg, sizeof(msg));
    if (msg[0] != '\0') {
        SysLogLocator::systemLog()->appendLog(msg, LogMsgType::Error);
        optFree(&mOPTHandle);
        return false;
    }

        for(OptionItem item: items) {
            qDebug() << QString("[%1=%2]").arg(item.key).arg(item.value);
        }
        qDebug();

    if (!optReadDefinition(mOPTHandle, QDir(mOption->getOptionDefinitionPath()).filePath(mOption->getOptionDefinitionFile()).toLatin1())) {
        optResetAllRecent(mOPTHandle);
        for(OptionItem item: items) {
            OptionDefinition optDef = mOption->getOptionDefinition( item.key );
//            optResetNr(mOPTHandle, number);
            QString value = item.value;
            switch(optDef.dataType) {
            case optDataInteger: {
                qDebug() << QString("%1=%2(int)").arg(item.key).arg(item.value.toInt());
                bool isCorrectDataType = false;
                int n = value.toInt(&isCorrectDataType);
                if (isCorrectDataType) {
                    optSetIntNr(mOPTHandle, optDef.number, n);
                } else  if (item.value.compare("maxint", Qt::CaseInsensitive)==0) {
                          optSetIntNr(mOPTHandle, optDef.number, OPTION_VALUE_MAXINT);
                } else if (value.compare("minint", Qt::CaseInsensitive)==0) {
                          optSetIntNr(mOPTHandle, optDef.number, OPTION_VALUE_MININT);
                } else {
                    optSetStrNr(mOPTHandle, optDef.number, value.toLatin1());
                }
                break;
            }
            case optDataDouble: {
                qDebug() << QString("%1=%2(int/double : double)").arg(item.key).arg(item.value);
                bool isCorrectDataType = false;
                double d = value.toDouble(&isCorrectDataType);
                if (isCorrectDataType) {
                    optSetDblNr(mOPTHandle, optDef.number, d);
                } else if (value.compare("maxdouble", Qt::CaseInsensitive)==0) {
                          optSetDblNr(mOPTHandle, optDef.number, OPTION_VALUE_MAXDOUBLE);
                } else if (value.compare("mindouble", Qt::CaseInsensitive)==0) {
                          optSetDblNr(mOPTHandle, optDef.number, OPTION_VALUE_MINDOUBLE);
//                } else if (value.compare("eps", Qt::CaseInsensitive)==0) {
                } else {
                    optSetStrNr(mOPTHandle, optDef.number, value.toLatin1());
                }
                break;
            }
            case optDataString: {
                qDebug() << QString("%1=%2(string)").arg(item.key).arg(item.value);
                optSetStrNr(mOPTHandle, optDef.number, item.value.toLatin1());
                break;
            }
            case optDataStrList: {
                qDebug() << QString("%1=%2(strlist)").arg(item.key).arg(item.value);
                optSetStrNr(mOPTHandle, optDef.number, item.value.toLatin1());
                break;
            }
            default:
                break;
            }
        }
        optWriteParameterFile(mOPTHandle, absoluteFilepath.toLatin1());
    }
    bool hasBeenLogged = logMessage(mOPTHandle);
    optFree(&mOPTHandle);
    return !hasBeenLogged;
}

void OptionTokenizer::validateOption(QList<OptionItem> &items)
{
   for(OptionItem& item : items) {
       if (mOption->isDoubleDashedOption(item.key)) { // double dashed parameter
           if ( mOption->isDoubleDashedOptionNameValid( mOption->getOptionKey(item.key)) )
               item.error = OptionErrorType::No_Error;
           else
              item.error = OptionErrorType::Invalid_Key;
           continue;
       }
       if (mOption->isValid(item.key) || mOption->isASynonym(item.key)) { // valid option
           if (mOption->isDeprecated(item.key)) { // deprecated option
               item.error = OptionErrorType::Deprecated_Option;
           } else { // valid and not deprected Option
               item.error = mOption->getValueErrorType(item.key, item.value);
           }
       } else { // invalid option
           item.error = OptionErrorType::Invalid_Key;
       }
   }
}

Option *OptionTokenizer::getOption() const
{
    return mOption;
}

AbstractSystemLogger *OptionTokenizer::logger()
{
    if (!mOptionLogger) return mNullLogger;
    return mOptionLogger;
}

void OptionTokenizer::provideLogger(AbstractSystemLogger *optionLogEdit)
{
    mOptionLogger = optionLogEdit;
}

void OptionTokenizer::formatLineEdit(QLineEdit* lineEdit, const QList<OptionError> &errorList) {
    QString errorMessage = "";
    QList<QInputMethodEvent::Attribute> attributes;
    foreach(const OptionError err, errorList)   {
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;
        int start = err.formatRange.start - lineEdit->cursorPosition();
        int length = err.formatRange.length;
        QVariant value = err.formatRange.format;
        attributes.append(QInputMethodEvent::Attribute(type, start, length, value));

        if (!err.message.isEmpty())
            errorMessage.append("\n    " + err.message);
    }

    if (!errorMessage.isEmpty()) {
        errorMessage.prepend("Error: Parameter error(s)");
        lineEdit->setToolTip(errorMessage);
    } else {
        lineEdit->setToolTip("");
    }
    QInputMethodEvent event(QString(), attributes);
    QCoreApplication::sendEvent(lineEdit, &event);
}

} // namespace option
} // namespace studio
} // namespace gams
