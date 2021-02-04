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
