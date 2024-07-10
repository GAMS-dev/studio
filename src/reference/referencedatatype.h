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
#ifndef REFERENCEDATATYPE_H
#define REFERENCEDATATYPE_H

#include <QString>
#include <QStringList>

namespace gams {
namespace studio {
namespace reference {

///
/// \brief The ReferenceDataType class defines all reference types
///  used by the Reference obejct. The instances are accessed via
/// static functions. On the first usage, the list is initialized.
///
class ReferenceDataType
{
public:
    enum ReferenceType {
        Unknown = 0,
        Declare = 1,
        Define = 2,
        Assign = 3,
        ImplicitAssign = 4,
        Reference = 5,
        Index = 6,
        Control = 7
    };

    ReferenceType type() const;
    QString name() const;

    QString description() const;

    bool operator ==(const ReferenceDataType& refDataType) const;
    bool operator !=(const ReferenceDataType& refDataType) const;
    bool operator ==(const ReferenceDataType::ReferenceType& type) const;
    bool operator !=(const ReferenceDataType::ReferenceType& type) const;

    static const QList<ReferenceDataType*> list();
    static ReferenceType typeFrom(const QString& name);
    static ReferenceDataType& from(const QString& name);
    static ReferenceDataType& from(const ReferenceType type);

private:
    static void clear();
    ReferenceDataType(ReferenceType type, const QString& name, const QString &typeDescription);

    const ReferenceType mType;
    const QStringList mName;
    const QString mDescription;

    static QList<ReferenceDataType*> mList;
    static ReferenceDataType*  mUnknown;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // REFERENCEDATATYPE_H
