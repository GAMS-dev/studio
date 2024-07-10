/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef OPTIONTOKENIZER_H
#define OPTIONTOKENIZER_H

#include <QTextLayout>
#include <QLineEdit>

#include "option.h"
#include "commonpaths.h"
#include "editors/abstractsystemlogger.h"

namespace gams {
namespace studio {
namespace option {


struct OptionError {
    OptionError() { }
    OptionError(const QTextLayout::FormatRange &fr, const QString &m):
        formatRange(fr), message(m) { }
    OptionError(const QTextLayout::FormatRange &fr, const QString &m, bool w):
         formatRange(fr), message(m), warning(w) { }
    OptionError(const QTextLayout::FormatRange &fr, const QString &m, bool w, int id):
         formatRange(fr), message(m), warning(w), optionId(id) { }

    QTextLayout::FormatRange formatRange;
    QString message;
    bool warning =  false;
    int optionId = -1;
};

class OptionTokenizer : public QObject
{
    Q_OBJECT

public:

    OptionTokenizer(const QString &optionDefFileName, const QString &optionDefFilePath=CommonPaths::systemDir());
    ~OptionTokenizer();

    QList<OptionItem> tokenize(const QString &commandLineStr);
    QList<OptionItem> tokenize(const QString &commandLineStr, const QList<QString> &disabledOption);
    QList<OptionError> format(const QList<OptionItem> &items);
    QList<OptionErrorType> validate(ParamConfigItem * item);
    QString normalize(const QString &commandLineStr);
    QString normalize(const QList<OptionItem> &items);

    QTextCharFormat invalidKeyFormat() const;
    QTextCharFormat invalidValueFormat() const;
    QTextCharFormat deprecateOptionFormat() const;

    void setInvalidKeyFormat(const QTextCharFormat &invalidKeyFormat);
    void setInvalidValueFormat(const QTextCharFormat &invalidValueFormat);
    void setDeprecateOptionFormat(const QTextCharFormat &deprecateOptionFormat);
    void setDeactivatedOptionFormat(const QTextCharFormat &deactivatedOptionFormat);

    QString formatOption(const SolverOptionItem *item);
    bool getOptionItemFromStr(SolverOptionItem *item, bool firstTime, const QString &str);
    bool updateOptionItem(const QString &key, const QString &value, const QString &text, SolverOptionItem* item);

    QList<SolverOptionItem *> readOptionFile(const QString &absoluteFilePath, const QString &encodingName);
    bool writeOptionFile(const QList<SolverOptionItem *> &items, const QString &absoluteFilepath, const QString &encodingName);

    void validateOption(QList<OptionItem> &items);
    void validateOption(QList<SolverOptionItem *> &items);
    void validateOption(QList<ParamConfigItem *> &items);

    Option *getOption() const;

    AbstractSystemLogger* logger();
    void provideLogger(AbstractSystemLogger* optionLogEdit);

    QChar getEOLCommentChar() const;
    bool isValidLineCommentChar(const QChar& ch);
    bool isValidEOLCommentChar(const QChar& ch);

    static QString keyGeneratedStr;
    static QString valueGeneratedStr;
    static QString commentGeneratedStr;

public slots:
    void formatTextLineEdit(QLineEdit* lineEdit, const QString &commandLineStr);
    void formatItemLineEdit(QLineEdit* lineEdit, const QList<gams::studio::option::OptionItem> &optionItems);

private:
    Option* mOption = nullptr;
    optHandle_t mOPTHandle;
    bool mOPTAvailable = false;
    QStringList mLineComments;
    QChar mEOLCommentChar = QChar();

    QTextCharFormat mInvalidKeyFormat;
    QTextCharFormat mInvalidValueFormat;
    QTextCharFormat mMissingValueFormat;
    QTextCharFormat mDeprecateOptionFormat;
    QTextCharFormat mDeactivatedOptionFormat;
    QTextCharFormat mDuplicateOptionFormat;

    AbstractSystemLogger* mOptionLogger = nullptr;
    static AbstractSystemLogger* mNullLogger;
    static QRegularExpression mRexVersion;

    OptionErrorType getErrorType(optHandle_t &mOPTHandle);
    bool logMessage(optHandle_t &mOPTHandle);
    OptionErrorType logAndClearMessage(optHandle_t &OPTHandle, bool logged = true);

    QString getKeyFromStr(const QString &line, const QString &hintKey);
    QString getDoubleValueFromStr(const QString &line, const QString &hintKey, const QString &hintValue);
    QString getValueFromStr(const QString &line, const int itype, const int ioptsubtype, const QString &hintKey, const QString &hintValue);
    QString getEOLCommentFromStr(const QString &line, const QString &hintKey, const QString &hintValue);
    QString getQuotedStringValue(const QString &line, const QString &value);
    int getEOLCommentCharIndex(const QString &text);
    void parseOptionString(const QString &text, QString &keyStr, QString &valueStr, QString &commentStr);

    void offsetWhiteSpaces(const QString &str, int &offset, const int length);
    void offsetKey(const QString &str,  QString &key, int &keyPosition, int &offset, const int length);
    void offsetAssignment(const QString &str, int &offset, const int length);
    void offsetValue(const QString &str, QString &value, int &valuePosition, int &offset, const int length);

    void formatLineEdit(QLineEdit* lineEdit, const QList<OptionError> &errorList);
};

} // namespace option
} // namespace studio
} // namespace gams

#endif // OPTIONTOKENIZER_H
