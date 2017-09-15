#ifndef FILEGROUPCONTEXT_H
#define FILEGROUPCONTEXT_H

#include "filesystemcontext.h"

namespace gams {
namespace ide {

class FileGroupContext : public FileSystemContext
{
    Q_OBJECT
public:

protected:
    friend class FileRepository;
    FileGroupContext(FileGroupContext *parent, int id, QString name, QString location, bool isGist);
};

} // namespace ide
} // namespace gams

#endif // FILEGROUPCONTEXT_H
