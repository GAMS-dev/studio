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

#ifndef COMMON_H
#define COMMON_H

namespace gams {
namespace studio {

typedef int FileId;
typedef int NodeId;
typedef int TextMarkId;

enum struct NameModifier {
    Raw,
    EditState
};
enum struct NodeType {
    File,
    Group,
    Log
};
enum FileKind {
    None,
    Gsp,
    Gms,
    Txt,
    Lst,
    Lxi,
    Log,
    Gdx,
    Ref,
};


}
}

#endif // COMMON_H
