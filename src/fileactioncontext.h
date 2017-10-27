#ifndef FILEACTIONCONTEXT_H
#define FILEACTIONCONTEXT_H

#include "filesystemcontext.h"

namespace gams {
namespace studio {

class FileActionContext : public FileSystemContext
{
    Q_OBJECT
public:
    void trigger();
    void setLocation(const QString& location);
    virtual QIcon icon();

private:
    friend class FileRepository;
    FileActionContext(int id, QAction* action);

private:
    QAction *mAction;
};

} // namespace studio
} // namespace gams

#endif // FILEACTIONCONTEXT_H
