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
#ifndef LOGPARSER_H
#define LOGPARSER_H

#include <QString>
#include <QtCore5Compat/QTextCodec>
#include "syntax/textmarkrepo.h"

namespace gams {
namespace studio {

class LogParser: public QObject
{
    Q_OBJECT
public:
    struct ErrorData {
        QString text;
        int lstLine = 0;
        int errNr = 0;
    };

    struct MarkData {
        void setMark(const QString &href, int lstline = 0) { hRef = href; lstLine = lstline; }
        void setErrMark(const QString &href, int errnr) { errRef = href; errNr = errnr; }
        bool hasMark() const { return !hRef.isEmpty() || hasErr(); }
        bool hasErr() const { return !errRef.isEmpty() || errNr; }
        QString hRef;
        int lstLine = 0;
        QString errRef;
        int errNr = 0;
    };

    struct MarksBlockState {
        MarkData marks;
        ErrorData errData;
        QString switchLst;
        bool deep = false;
    };

public:
    LogParser(QTextCodec *codec);
    QTextCodec *codec() const;
    void setCodec(QTextCodec *codec);
    QString parseLine(const QByteArray &data, QString &line, bool &hasError, MarksBlockState &mbState);
    void quickParse(const QByteArray &data, int start, int end, QString &line, int &linkStart, int &lstLine);


signals:
    void setErrorText(int lstLine, QString text);
    void hasFile(QString fName, bool &exists);

private:
    QString extractLinks(const QString &line, bool &hasError, MarksBlockState &mbState);

    QString mDirectory;
    QTextCodec *mCodec;
//    mutable QStringEncoder encode;
//    QStringConverter::Encoding mEncoding;
};

} // namespace studio
} // namespace gams

#endif // LOGPARSER_H
