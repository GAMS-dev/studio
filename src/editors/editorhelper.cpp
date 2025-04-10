/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "editorhelper.h"

namespace gams {
namespace studio {

void EditorHelper::nextWord(int offset,int &pos, const QString &text)
{
    if (text.length() == 0) {
        // short line
        pos++;
        return;
    } else if (pos+offset < text.length()) {
        // navigating in a line
        if (pos + offset > text.length()) return;
        bool stop = false;

        while (++pos + offset < text.length()) {
            // stop if last char is not letter or number
            stop = (pos + offset > 0) && !text.at(pos + offset - 1).isLetterOrNumber();
            if (text.at(pos + offset).isLetterOrNumber() && stop) return;
        }
    } else {
        // reached end of line
        pos++;
    }
}

void EditorHelper::prevWord(int offset, int &pos, const QString &text)
{
    if (pos + offset > text.length()) {
        pos--;
        return;
    }
    if (pos + offset > 0) {
        --pos;
        if (pos+offset == 0) return;
        bool last = false;

        while (--pos + offset > 0) {
            last = !text.at(pos + offset + 1).isLetterOrNumber();
            if (!text.at(pos+offset).isLetterOrNumber() && !last) {
                ++pos;
                return;
            }
        }
    }
}

QString EditorHelper::formatTime(qreal timeSec, bool compact)
{
    QString res;
    if (compact || timeSec < 10.) {
        int roundSec = qRound(timeSec);
        int roundMin = qRound(timeSec/60) % 60;
        res = timeSec < 10.    ? QString("%1 s").arg(timeSec,  0, 'f', 3)
            : timeSec < 60.    ? QString("%1 s").arg(timeSec,  0, 'f', 2)
            : timeSec < 3600.  ? QString("%1:%2 m").arg(int(timeSec/60)).arg(roundSec % 60, 2, 10, QChar('0'))
            : timeSec < 86400. ? QString("%1:%2 h").arg(int(timeSec/3600)).arg(roundMin, 2, 10, QChar('0'))
                               : QString("%1 h").arg(int(timeSec/3600));
    } else {
        int msec = qRound(timeSec*1000. - int(timeSec)*1000);
        res = timeSec < 60.    ? QString("%1 s").arg(timeSec,  0, 'f', 3)
            : timeSec < 3600.  ? QString("%1:%2.%3 m").arg(int(timeSec/60)).arg(int(timeSec) % 60, 2, 10, QChar('0')).arg(msec)
            : QString("%1:%2:%3.%4 h").arg(int(timeSec/3600)).arg(int(timeSec/60) % 60, 2, 10, QChar('0'))
                                      .arg(int(timeSec) % 60, 2, 10, QChar('0')).arg(msec);
    }
    return res;
}

QString EditorHelper::formatTime2(qreal timeSec, int &unitIndex, bool compact)
{
    QString res;

    if (unitIndex < 0) {
        unitIndex = timeSec < 60.    ? 0
                  : timeSec < 3600.  ? 1
                  : timeSec < 86400. ? 2 : 3;
    }
    switch (unitIndex) {
    case 0: // sec
        res = QString::number(timeSec, 'f', (!compact || timeSec < 10.) ? 3 : 2);
        break;
    case 1: // min
        if (compact)
            res = QString("%1:%2").arg(int(timeSec/60)).arg(qRound(timeSec) % 60, 2, 10, QChar('0'));
        else {
            int msec = qRound(timeSec*1000. - int(timeSec)*1000);
            res = QString("%1:%2.%3").arg(int(timeSec/60)).arg(int(timeSec) % 60, 2, 10, QChar('0')).arg(msec);
        }
        break;
    case 2: // hour
        if (compact)
            res = QString("%1:%2").arg(int(timeSec/3600)).arg(qRound(timeSec/60) % 60, 2, 10, QChar('0'));
        else {
            int msec = qRound(timeSec*1000. - int(timeSec)*1000);
            res = QString("%1:%2:%3.%4").arg(int(timeSec/3600)).arg(int(timeSec/60) % 60, 2, 10, QChar('0'))
                      .arg(int(timeSec) % 60, 2, 10, QChar('0')).arg(msec);
        }
        break;
    case 3: { // day
        qreal days = timeSec / 86400.;
        if (compact)
            res = QString::number(days, 'f', days < 10 ? 2 : days < 100 ? 1 : 0);
        else {
            int days = int(timeSec / 86400);
            qreal remain = timeSec - days * 86400;
            res = QString("%1 %2:%3:%4").arg(days).arg(int(remain/3600)).arg(int(remain/60) % 60, 2, 10, QChar('0'))
            .arg(int(remain) % 60, 2, 10, QChar('0'));
        }
    } break;
    default:
        res = "wrong unit";
    }
    return res;
}

QString EditorHelper::timeUnit(qreal timeSec)
{
    QString res;
    res = timeSec < 60.    ? QString("sec")
        : timeSec < 3600.  ? QString("min")
        : timeSec < 86400. ? QString("hour")
                           : QString("days");
    return res;
}

QString EditorHelper::formatMemory(size_t bytes, bool compact)
{
    QString res;
    qreal rbytes = qreal(bytes);
    if (compact || bytes < 1000) {
        res = (bytes < 1000            ? QString("%1 B ").arg(rbytes)
             : bytes < 100000          ? QString("%1 KB").arg(rbytes*.001, 0, 'f', 2)
             : bytes < 1000000         ? QString("%1 KB").arg(rbytes*.001, 0, 'f', 1)
             : bytes < 100000000       ? QString("%1 MB").arg(rbytes*.000001, 0, 'f', 2)
             : bytes < 1000000000      ? QString("%1 MB").arg(rbytes*.000001, 0, 'f', 1)
             : bytes < 100000000000    ? QString("%1 GB").arg(rbytes*.000000001, 0, 'f', 2)
             : bytes < 1000000000000   ? QString("%1 GB").arg(rbytes*.000000001, 0, 'f', 1)
             : bytes < 100000000000000 ? QString("%1 TB").arg(rbytes*.000000000001, 0, 'f', 2)
                                       : QString("%1 TB").arg(rbytes*.000000000001, 0, 'f', 1)).rightJustified(8, ' ');
    } else {
        res = (bytes < 1000000       ? QString("%1 KB").arg(rbytes*.001, 0, 'f', 3)
             : bytes < 1000000000    ? QString("%1 MB").arg(rbytes*.000001, 0, 'f', 3)
             : bytes < 1000000000000 ? QString("%1 GB").arg(rbytes*.000000001, 0, 'f', 3)
                                     : QString("%1 TB").arg(rbytes*.000000000001, 0, 'f', 3)).rightJustified(10, ' ');
    }
    return res;
}

QString EditorHelper::formatMemory2(size_t bytes, int &unitIndex, bool compact)
{
    QString res;
    if (unitIndex < 0) {
        unitIndex = bytes < 1000           ? 0
                  : bytes < 1000000        ? 1
                  : bytes < 1000000000     ? 2
                  : bytes < 1000000000000  ? 3 : 4;
    }
    switch (unitIndex) {
    case 0:
        res = QString::number(bytes).rightJustified(3, ' ');
        break;
    case 1:
        if (compact)
            res = QString::number(qreal(bytes)/1000., 'f', 3).leftJustified(5, ' ', true);
        else
            res = QString::number(qreal(bytes)/1000., 'f', 3);
        break;
    case 2:
        if (compact)
            res = QString::number(qreal(bytes)/1000000., 'f', 3).leftJustified(5, ' ', true);
        else
            res = QString::number(qreal(bytes)/1000000., 'f', 3);
        break;
    case 3:
        if (compact)
            res = QString::number(qreal(bytes)/1000000000., 'f', 3).leftJustified(5, ' ', true);
        else
            res = QString::number(qreal(bytes)/1000000000., 'f', 3);
        break;
    case 4:
        if (compact)
            res = QString::number(qreal(bytes)/1000000000000., 'f', 3).leftJustified(5, ' ', true);
        else
            res = QString::number(qreal(bytes)/1000000000000., 'f', 3);
        break;
    default:
        break;
    }
    if (res.endsWith('.')) res = (compact ? " " : "") + res.left(res.length()-1);
    return res;
}

QString EditorHelper::memoryUnit(size_t bytes)
{
    QString res;
    res = bytes < 1000          ? QString("B")
        : bytes < 1000000       ? QString("KB")
        : bytes < 1000000000    ? QString("MB")
        : bytes < 1000000000000 ? QString("GB")
                                : QString("TB");
    return res;
}

QString EditorHelper::formatLoop(size_t loop, int digits)
{
    return QString("%3").arg(loop).rightJustified(digits + 2, ' ');
}

}
}
