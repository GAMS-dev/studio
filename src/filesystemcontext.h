#ifndef FILESYSTEMCONTEXT_H
#define FILESYSTEMCONTEXT_H

#include <QtCore>

namespace gams {
namespace ide {

class FileSystemContext : public QObject
{
    Q_OBJECT
public:
    virtual ~FileSystemContext();

    int id() const;
    bool isGist() const;
    virtual const QString name();
    void setName(const QString& name);

    const QString& location() const;
    bool matches(const QString& name, bool isGist) const;
    FileSystemContext* child(int index) const;
    FileSystemContext* parentEntry() const;
    int peekIndex(QString name, bool skipLast = false);
    bool active() const;

signals:
    void nameChanged(int id, QString newName);

protected:
    FileSystemContext(FileSystemContext* parent, int id, QString name, QString location, bool isGist);

protected:
    int mId;
    FileSystemContext* mParent;
    QString mName;
    QString mPath;
    bool mIsGist;
    bool mActive = false;

};

} // namespace ide
} // namespace gams

#endif // FILESYSTEMCONTEXT_H
