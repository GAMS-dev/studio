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
#ifndef SYMBOLDATATYPE_H
#define SYMBOLDATATYPE_H

#include <QString>
#include <QStringList>

namespace gams {
namespace studio {
namespace reference {

///
/// \brief The SymbolDataType class defines all symbol data types
///  used by the Reference obejct. The instances are accessed via
/// static functions. On the first usage, the list is initialized.
///
class SymbolDataType
{
public:
    enum SymbolType {
        Unknown = 0,
        Funct = 1,
        Set = 2,
        Acronym = 3,
        Parameter = 4,
        Variable = 5,
        Equation = 6,
        Model = 7,
        File  = 8,
        Macro = 9,
        FileUsed = 99,
        Unused = 100
    };

    SymbolType type() const;
    QString name() const;

    QStringList description() const;

    bool operator ==(const SymbolDataType& symbolDataType) const;
    bool operator !=(const SymbolDataType& symbolDataType) const;
    bool operator ==(const SymbolDataType::SymbolType& type) const;
    bool operator !=(const SymbolDataType::SymbolType& type) const;

    static const QList<SymbolDataType*> list();
    static SymbolType typeFrom(const QString& typeDescription);
    static SymbolDataType& from(const QString& typeDescription);
    static SymbolDataType& from(SymbolType type);

private:
    static void clear();
    SymbolDataType(SymbolType type, const QString& typeDescription);

    const SymbolType mType;
    const QStringList mDescription;

    static QList<SymbolDataType*> mList;
    static SymbolDataType* mUnknown;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // SYMBOLDATATYPE_H
