#ifndef GAMS_STUDIO_FILEICON_H
#define GAMS_STUDIO_FILEICON_H

#include <QIcon>
#include "filetype.h"

namespace gams {
namespace studio {

class FileIcon
{
    FileIcon();
public:
    static QIcon iconForFileKind(FileKind kind, bool isReadonly = false, bool isMain = false);
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_FILEICON_H
