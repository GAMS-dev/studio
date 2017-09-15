#include "filegroupcontext.h"

namespace gams {
namespace ide {

FileGroupContext::FileGroupContext(FileGroupContext* parent, int id, QString name, QString location, bool isGist)
    : FileSystemContext(parent, id, name, location, isGist)
{

}

} // namespace ide
} // namespace gams
