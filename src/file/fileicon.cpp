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
    if (kind == FileKind::Gms) return Scheme::icon(":/img/gams-w"+runMark);
    if (kind == FileKind::Gdx) return Scheme::icon(":/img/database");
    if (kind == FileKind::Ref) return Scheme::icon(":/img/ref-file");
    if (kind == FileKind::Opt) return Scheme::icon(":/img/option-file");
    if (kind == FileKind::Lst) return Scheme::icon(":/img/file-alt");
    if (kind == FileKind::Lxi) return Scheme::icon(":/img/file-alt");
    if (kind == FileKind::Log) return Scheme::icon(":/img/file-alt");
    if (kind == FileKind::Txt) return Scheme::icon(":/img/file-edit");
    if (kind == FileKind::TxtRO) return Scheme::icon(":/img/file-alt");
    if (!isReadonly) return Scheme::icon(":/img/file-edit");
    return Scheme::icon(":/img/file-alt"+runMark);

}

} // namespace studio
} // namespace gams
