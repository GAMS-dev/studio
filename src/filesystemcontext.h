#ifndef FILESYSTEMCONTEXT_H
#define FILESYSTEMCONTEXT_H

#include <QtGui>

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
    virtual void setLocation(const QString& location);
    virtual bool active();

    bool matches(const QString& name, bool isGist) const;
    FileSystemContext* child(int index) const;
    FileSystemContext* parentEntry() const;
    int peekIndex(QString name, bool skipLast = false);

signals:
    void nameChanged(int id, QString newName);

protected:
    FileSystemContext(FileSystemContext* parent, int id, QString name, QString location, bool isGist);

protected:
    int mId;
    FileSystemContext* mParent;
    QString mName;
    QString mLocation;
    bool mIsGist;
    bool mActive = false;

};

} // namespace ide
} // namespace gams

#endif // FILESYSTEMCONTEXT_H
