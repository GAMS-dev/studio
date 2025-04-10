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
#include "fileicon.h"
#include "theme.h"

namespace gams {
namespace studio {

FileIcon::FileIcon()
{

}

QIcon FileIcon::iconForFileKind(FileKind kind, bool isReadonly, bool isActive, QIcon::Mode mode, int alpha)
{
    QString runMark = isActive ? "-run" : "";
    // Efi -> currently default "file-edit"
    if (kind == FileKind::GCon) return Theme::icon(":/img/file-connect", mode, alpha);
    if (kind == FileKind::Gdx) return Theme::icon(":/img/database", mode, alpha);
    if (kind == FileKind::Gms) return Theme::icon(":/img/gams-w"+runMark, mode, alpha);
    // Gsp -> fetched directly
    if (kind == FileKind::Guc) return Theme::icon(":/img/gams-config-file", mode, alpha);
    if (kind == FileKind::Log) return Theme::icon(":/img/file-alt", mode, alpha);
    if (kind == FileKind::Lst) return Theme::icon(":/img/file-alt", mode, alpha);
    if (kind == FileKind::Lxi) return Theme::icon(":/img/file-alt", mode, alpha);
    if (kind == FileKind::Opt) return Theme::icon(":/img/option-file", mode, alpha);
    if (kind == FileKind::Pf) return Theme::icon(":/img/file-param"+runMark, mode, alpha);
    if (kind == FileKind::Ref) return Theme::icon(":/img/ref-file", mode, alpha);
    if (kind == FileKind::Txt) return Theme::icon(":/img/file-edit", mode, alpha);
    if (kind == FileKind::TxtRO) return Theme::icon(":/img/file-alt", mode, alpha);
    if (!isReadonly) return Theme::icon(":/img/file-edit", mode, alpha);
    return Theme::icon(":/img/file-alt", mode, alpha);

}

} // namespace studio
} // namespace gams
