#ifndef FILEGROUPCONTEXT_H
#define FILEGROUPCONTEXT_H

#include "filesystemcontext.h"

namespace gams {
namespace ide {

class FileGroupContext : public FileSystemContext
{
    Q_OBJECT
public:
    ~FileGroupContext();
    void setFlag(ContextFlag flag, bool value = true);
    void unsetFlag(ContextFlag flag);

    int childCount();
    int indexOf(FileSystemContext *child);
    FileSystemContext* childEntry(int index);

signals:
    void contentChanged(int id, QDir fileInfo);

public slots:
    void directoryChanged(const QString &path);

protected:
    friend class FileRepository;
    friend class FileSystemContext;

    FileGroupContext(FileGroupContext *parent, int id, QString name, QString location, bool isGist);
    int peekIndex(const QString &name, bool* exactMatch = nullptr);
    void insertChild(FileSystemContext *child);
    void insertChild(int pos, FileSystemContext *child);
    void removeChild(FileSystemContext *child);
    void checkFlags();

private:
    QList<FileSystemContext*> mChildList;
};

} // namespace ide
} // namespace gams

#endif // FILEGROUPCONTEXT_H
