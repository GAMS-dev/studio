/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
 */
#include "fileicon.h"
#include "theme.h"

namespace gams {
namespace studio {

FileIcon::FileIcon()
{

}

QIcon FileIcon::iconForFileKind(FileKind kind, bool isReadonly, bool isMain)
{
    QString runMark = isMain ? "-run" : "";
    if (kind == FileKind::Gms) return Theme::icon(":/img/gams-w"+runMark, true);
    if (kind == FileKind::Gdx) return Theme::icon(":/img/database", true);
    if (kind == FileKind::Ref) return Theme::icon(":/img/ref-file", true);
    if (kind == FileKind::Opt) return Theme::icon(":/img/option-file", true);
    if (kind == FileKind::Guc) return Theme::icon(":/img/gams-config-file", true);
    if (kind == FileKind::Lst) return Theme::icon(":/img/file-alt", true);
    if (kind == FileKind::Lxi) return Theme::icon(":/img/file-alt", true);
    if (kind == FileKind::Log) return Theme::icon(":/img/file-alt", true);
    if (kind == FileKind::Txt) return Theme::icon(":/img/file-edit", true);
    if (kind == FileKind::TxtRO) return Theme::icon(":/img/file-alt", true);
    if (!isReadonly) return Theme::icon(":/img/file-edit", true);
    return Theme::icon(":/img/file-alt", true);

}

} // namespace studio
} // namespace gams
