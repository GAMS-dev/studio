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

#ifndef ENCODING_H
#define ENCODING_H

#include <QString>

namespace gams {
namespace studio {

class Encoding
{
public:
    Encoding();

    static QStringDecoder createDecoder(const QString &encodingName);
    static QStringEncoder createEncoder(const QString &encodingName);
    static QString name(int mib);
    static int nameToOldMib(QString encName);
    static QString toUtf16(QByteArray data, QByteArray encoding = QByteArray());
    static QString defaultEncoding(int fallbackMib = -1);
    static void setDefaultEncoding(const QString &encoding);
private:
    static QString mDefaultEncoding;
};

} // namespace studio
} // namespace gams

#endif // ENCODING_H
