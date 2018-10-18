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
#ifndef COMMANDLINETOKENIZER_H
#define COMMANDLINETOKENIZER_H

#include <QTextLayout>
#include <QLineEdit>
#include "option.h"
#include "locators/abstractsystemlogger.h"

namespace gams {
namespace studio {
namespace option {


struct OptionError {
    OptionError() { }
    OptionError(QTextLayout::FormatRange fr, QString m):
         formatRange(fr), message(m) { }

    QTextLayout::FormatRange formatRange;
    QString message;
};

class OptionTokenizer : public QObject
{
    Q_OBJECT

public:

    OptionTokenizer(const QString &optionFileName);
    ~OptionTokenizer();

    QList<OptionItem> tokenize(const QString &commandLineStr);
    QList<OptionItem> tokenize(const QString &commandLineStr, const QList<QString> &disabledOption);
    QList<OptionError> format(const QList<OptionItem> &items);
    QString normalize(const QString &commandLineStr);
    QString normalize(const QList<OptionItem> &items);

    QTextCharFormat invalidKeyFormat() const;
    QTextCharFormat invalidValueFormat() const;
    QTextCharFormat deprecateOptionFormat() const;

    void setInvalidKeyFormat(const QTextCharFormat &invalidKeyFormat);
    void setInvalidValueFormat(const QTextCharFormat &invalidValueFormat);
    void setDeprecateOptionFormat(const QTextCharFormat &deprecateOptionFormat);
    void setDeactivatedOptionFormat(const QTextCharFormat &deactivatedOptionFormat);

    QList<OptionItem> readOptionParameterFile(const QString &absoluteFilePath);
    bool writeOptionParameterFile(const QList<OptionItem> &items, const QString &absoluteFilepath);

    void validateOption(QList<OptionItem> &items);

    Option *getOption() const;

    AbstractSystemLogger* logger();
    void provideLogger(AbstractSystemLogger* optionLogEdit);

public slots:
    void formatTextLineEdit(QLineEdit* lineEdit, const QString &commandLineStr);
    void formatItemLineEdit(QLineEdit* lineEdit, const QList<OptionItem> &optionItems);

private:
    Option* mOption = nullptr;

    QTextCharFormat mInvalidKeyFormat;
    QTextCharFormat mInvalidValueFormat;
    QTextCharFormat mDeprecateOptionFormat;
    QTextCharFormat mDeactivatedOptionFormat;

    AbstractSystemLogger* mOptionLogger = nullptr;
    static AbstractSystemLogger* mNullLogger;

    bool logMessage(optHandle_t &mOPTHandle);

    void offsetWhiteSpaces(QStringRef str, int &offset, const int length);
    void offsetKey(QStringRef str,  QString &key, int &keyPosition, int &offset, const int length);
    void offsetAssignment(QStringRef str, int &offset, const int length);
    void offsetValue(QStringRef str, QString &value, int &valuePosition, int &offset, const int length);

    void formatLineEdit(QLineEdit* lineEdit, const QList<OptionError> &errorList);
};

} // namespace option
} // namespace studio
} // namespace gams

#endif // COMMANDLINETOKENIZER_H
