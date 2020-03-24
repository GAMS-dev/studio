#include "fileicon.h"
#include "scheme.h"

namespace gams {
namespace studio {

FileIcon::FileIcon()
{

}

QIcon FileIcon::iconForFileKind(FileKind kind, bool isReadonly, bool isMain)
{
    QString runMark = isMain ? "-run" : "";
    if (kind == FileKind::Gms) return Scheme::icon(":/img/gams-w"+runMark, true);
    if (kind == FileKind::Gdx) return Scheme::icon(":/img/database", true);
    if (kind == FileKind::Ref) return Scheme::icon(":/img/ref-file", true);
    if (kind == FileKind::Opt) return Scheme::icon(":/img/option-file", true);
    if (kind == FileKind::GCfg) return Scheme::icon(":/img/option-file", true);
    if (kind == FileKind::Lst) return Scheme::icon(":/img/file-alt", true);
    if (kind == FileKind::Lxi) return Scheme::icon(":/img/file-alt", true);
    if (kind == FileKind::Log) return Scheme::icon(":/img/file-alt", true);
    if (kind == FileKind::Txt) return Scheme::icon(":/img/file-edit", true);
    if (kind == FileKind::TxtRO) return Scheme::icon(":/img/file-alt", true);
    if (!isReadonly) return Scheme::icon(":/img/file-edit", true);
    return Scheme::icon(":/img/file-alt"+runMark, true);

}

} // namespace studio
} // namespace gams
