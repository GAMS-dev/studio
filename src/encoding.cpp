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

#include "encoding.h"
#include "settings.h"

#include <QHash>

namespace gams {
namespace studio {

QHash<int, QStringList> C_OldMibToName = {
    {0,{"System"}},
    {4,{"ISO-8859-1","latin1","CP819","IBM819","iso-ir-100","csISOLatin1"}},
    {5,{"ISO-8859-2","latin2","iso-ir-101","csISOLatin2"}},
    {6,{"ISO-8859-3","latin3","iso-ir-109","csISOLatin3"}},
    {7,{"ISO-8859-4","latin4","iso-ir-110","csISOLatin4"}},
    {8,{"ISO-8859-5","cyrillic","iso-ir-144","csISOLatinCyrillic"}},
    {10,{"ISO-8859-7","ECMA-118","greek","iso-ir-126","csISOLatinGreek"}},
    {12,{"ISO-8859-9","iso-ir-148","latin5","csISOLatin5"}},
    {13,{"ISO-8859-10","iso-ir-157","latin6","ISO-8859-10:1992","csISOLatin6"}},
    {17,{"Shift_JIS","SJIS","MS_Kanji"}},
    {18,{"EUC-JP"}},
    {38,{"EUC-KR"}},
    {39,{"ISO-2022-JP","JIS7"}},
    {82,{"ISO-8859-6","ISO-8859-6-I","ECMA-114","ASMO-708","arabic","iso-ir-127","csISOLatinArabic"}},
    {85,{"ISO-8859-8","ISO 8859-8-I","iso-ir-138","hebrew","csISOLatinHebrew"}},
    {106,{"UTF-8"}},
    {109,{"ISO-8859-13"}},
    {110,{"ISO-8859-14","iso-ir-199","latin8","iso-celtic"}},
    {111,{"ISO-8859-15","latin9"}},
    {112,{"ISO-8859-16","iso-ir-226","latin10"}},
    {113,{"GBK","CP936","MS936","windows-936"}},
    {114,{"GB18030"}},
    {1013,{"UTF-16BE"}},
    {1014,{"UTF-16LE"}},
    {1015,{"UTF-16"}},
    {1017,{"UTF-32"}},
    {1018,{"UTF-32BE"}},
    {1019,{"UTF-32LE"}},
    {2004,{"hp-roman8","roman8","csHPRoman8"}},
    {2009,{"IBM850","CP850","csPC850Multilingual"}},
    {2025,{"GB2312"}},
    {2026,{"Big5","Big5-ETen","CP950"}},
    {2027,{"macintosh","Apple Roman","MacRoman"}},
    {2084,{"KOI8-R","csKOI8R"}},
    {2086,{"IBM866","CP866","csIBM866"}},
    {2088,{"KOI8-U","KOI8-RU"}},
    {2101,{"Big5-HKSCS"}},
    {2107,{"TSCII"}},
    {2250,{"windows-1250","CP1250"}},
    {2251,{"windows-1251","CP1251"}},
    {2252,{"windows-1252","CP1252"}},
    {2253,{"windows-1253","CP1253"}},
    {2254,{"windows-1254","CP1254"}},
    {2255,{"windows-1255","CP1255"}},
    {2256,{"windows-1256","CP1256"}},
    {2257,{"windows-1257","CP1257"}},
    {2258,{"windows-1258","CP1258"}},
    {2259,{"TIS-620","ISO 8859-11"}},
    };

QHash<QString, int> invertHash()
{
    QHash<QString, int> res;
    QHashIterator<int, QStringList> i(C_OldMibToName);
    while (i.hasNext()) {
        i.next();
        for (const QString &name : i.value())
            res.insert(name.toLower(), i.key());
    }
    return res;
}

QHash<QString, int> C_NameToOldMib = invertHash();

QString Encoding::mDefaultEncoding = "";

// =====================================================================================================

Encoding::Encoding() {}

QStringDecoder Encoding::createDecoder(const QString &encodingName)
{
    QStringDecoder decoder = encodingName.isEmpty() ? QStringDecoder(defaultEncoding().toLatin1())
                                                    : QStringDecoder(encodingName.toLatin1());
    if (!decoder.isValid())
        decoder = QStringDecoder(QStringDecoder::Latin1);
    return decoder;
}

QStringEncoder Encoding::createEncoder(const QString &encodingName)
{
    QStringEncoder encoder = encodingName.isEmpty() ? QStringEncoder(defaultEncoding().toLatin1())
                                                    : QStringEncoder(encodingName.toLatin1());
    if (!encoder.isValid())
        encoder = QStringEncoder(QStringDecoder::Latin1);
    return encoder;
}

QString Encoding::name(int mib)
{
    if (!C_OldMibToName.contains(mib)) return "";
    QStringList list = C_OldMibToName.value(mib);
    return list.first();
}

int Encoding::nameToOldMib(QString encName)
{
    return C_NameToOldMib.value(encName.toLower(), -1);
}


QString Encoding::toUtf16(QByteArray data, QByteArray encoding)
{
    if (encoding.isEmpty())
        return data;
    QStringDecoder dec(encoding);
    return dec.decode(data);
}

QString Encoding::defaultEncoding(int fallbackMib)
{
    QString encoding = mDefaultEncoding;
    if (encoding.isEmpty()) {
        encoding = fallbackMib < 0 ? QStringConverter::nameForEncoding(QStringConverter::Utf8)
                                   : name(fallbackMib);
    }
    return encoding;
}

void Encoding::setDefaultEncoding(const QString &encoding)
{
    mDefaultEncoding = encoding;
}

} // namespace studio
} // namespace gams
