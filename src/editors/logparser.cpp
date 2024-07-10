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
#include "logparser.h"
//#include "file.h"
#include <QDir>
#include "logger.h"

namespace gams {
namespace studio {

LogParser::LogParser(QTextCodec *codec)
    : mCodec(codec)
{}

QTextCodec *LogParser::codec() const
{
    return mCodec;
}

void LogParser::setCodec(QTextCodec *codec)
{
    mCodec = codec;
}

QString LogParser::parseLine(const QByteArray &data, QString &line, bool &hasError, MarksBlockState &mbState)
{
    QTextCodec::ConverterState convState;
    if (mCodec) {
        line = mCodec->toUnicode(data.constData(), data.size(), &convState);
    }
    if (!mCodec || convState.invalidChars > 0) {
        QTextCodec* locCodec = QTextCodec::codecForLocale();
        line = locCodec->toUnicode(data.constData(), data.size(), &convState);
    }
    return extractLinks(line, hasError, mbState);
}

void LogParser::quickParse(const QByteArray &data, int start, int end, QString &line, int &linkStart, int &lstLine)
{
    linkStart = -1;
    if (end < start+7 || data.at(end-1) != ']') { // 7 = minimal size of a link [LST:1]
        line = data.mid(start, end-start);
        return;
    }
    // To be fast, this algorithm assumes that TWO links are always ERR followed by LST
    int inQuote = 0;
    int cutEnd = end;
    for (int i = end-2 ; i > start; --i) {
        // backwards-search to find first '[' of the link(s)
        switch (data.at(i)) {
        case '\'': if (inQuote != 2) inQuote = inQuote? 0 : 1; break;
        case '\"': if (inQuote != 1) inQuote = inQuote? 0 : 2; break;
        case '[':
            if (!inQuote) {
                if (data.at(i+4) == ':') {
                    cutEnd = i;
                    if (linkStart < 0) linkStart = i;
                    if (i-1 > start && data.at(i-1) == ']') {
                        // get the lstLine here
                        bool ok;
                        lstLine = data.mid(i+5, end-i-6).toInt(&ok);
                        if (!ok) lstLine = -1;
                        --i; // another link found
                    } else inQuote = -1;
                } else {
                    inQuote = -1;
                }
            }
            break;
        default:
            break;
        }
        if (inQuote < 0) break;
    }
    line = data.mid(start, cutEnd-start);

}

inline QString capture(const QString &line, int &a, int &b, const int offset, const QChar ch)
{
    a = b + offset;
    b = line.indexOf(ch, a);
    if (b < 0) b = line.length();
    return line.mid(a, b-a);
}

QString LogParser::extractLinks(const QString &line, bool &hasError, LogParser::MarksBlockState &mbState)
{
    mbState.marks = MarkData();
    hasError = false;
    if (!line.endsWith(']') || line.length() < 5 || line.startsWith('[')) return line;

    mbState.errData.errNr = 0;
    QString result;
    int posA = 0;
    int posB = 0;

    // quick check for error

    if (line.startsWith("*** Error ")) {
        hasError = true;
        mbState.errData.lstLine = -1;
        mbState.errData.text = "";

        posB = 0;
        if (line.mid(9, 9) == " at line ") {
            result = capture(line, posA, posB, 0, ':');
            mbState.errData.errNr = -1;
            if (posB+2 < line.length()) {
                int subLen = (line.contains('[') ? line.indexOf('['): line.length()) - (posB+2);
                mbState.errData.text = line.mid(posB+2, subLen);
            }
        } else {
            bool ok = false;
            posA = 9;
            while (posA < line.length() && (line.at(posA)<'0' || line.at(posA)>'9')) posA++;
            posB = posA;
            while (posB < line.length() && line.at(posB)>='0' && line.at(posB)<='9') posB++;
            int errNr = line.mid(posA, posB-posA).toInt(&ok);
            result = capture(line, posA, posB, -posB, '[');
            ++posB;
            int end = posB+1;
            end = line.indexOf(']', end);
            mbState.errData.errNr = (ok ? errNr : 0);
            mbState.marks.setErrMark(line.mid(posB, end-posB), mbState.errData.errNr);
            posB = end+1;
        }
    }

    // Now we should have a system output

    while (posA < line.length()) {
        result += capture(line, posA, posB, 0, '[');

        if (posB+5 < line.length()) {

            int start = posB+1;

            if (line.mid(posB+1,5) == "LS2:\"") {
                // adding a secondary LST file
                QString fName = QDir::fromNativeSeparators(capture(line, posA, posB, 6, '"'));
                mbState.switchLst = fName;
                ++posB;
                mbState.marks.setMark(line.mid(start, posB-start));
                ++posB;

                // LST:
            } else if (line.mid(posB+1,4) == "LST:" || line.mid(posB+1,4) == "LS2:") {
                int lineNr = capture(line, posA, posB, 5, ']').toInt();
                mbState.errData.lstLine = lineNr;
                mbState.marks.setMark(line.mid(start, posB-start), mbState.errData.lstLine);
                if (mbState.errData.errNr < 0) mbState.marks.errNr = mbState.errData.errNr;
                ++posB;

                // FIL + REF
            } else if (line.mid(posB+1,4) == "FIL:" || line.mid(posB+1,4) == "REF:") {
                QDir::fromNativeSeparators(capture(line, posA, posB, 6, ']'));
                mbState.marks.setMark(line.mid(start, posB-start));
                ++posB;

            } else if (line.mid(posB+1,4) == "DIR:") {
                QDir::fromNativeSeparators(capture(line, posA, posB, 6, ']'));
                mbState.marks.setMark(line.mid(start, posB-start));
                ++posB;

                // TIT
            } else if (line.mid(posB+1,4) == "TIT:") {
                return QString();
            } else {
                // no link reference: restore missing braces
                result += '['+capture(line, posA, posB, 1, ']')+']';
                posB++;
            }
        } else {
            if (posB < line.length()) result += line.right(line.length() - posB);
            break;
        }
    }
    return result;
}

} // namespace studio
} // namespace gams
