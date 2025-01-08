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
#ifndef GLBPARSER_H
#define GLBPARSER_H

#include "libraryitem.h"

#include <QTextStream>

namespace gams {
namespace studio {
namespace modeldialog {

class GlbParser
{
public:
    GlbParser();
    bool parseFile(const QString &glbFile);
    QList<LibraryItem> libraryItems() const;
    QString errorMessage() const;

private:
    QList<LibraryItem> mLibraryItems;
    QString mErrorMessage;
    bool checkListSize(const QStringList& list, int expectedSize);
    bool checkKey(const QString &key, const QString &expected);
    QString readLine(QTextStream& in);
    int mLineNr = 0;
    QString mGlbFile;
};

} // namespace modeldialog
} // namespace studio
} // namespace gams

#endif // GLBPARSER_H
