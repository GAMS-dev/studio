#ifndef FILESYSTEMCONTEXT_H
#define FILESYSTEMCONTEXT_H

#include <QtGui>

namespace gams {
namespace ide {

class FileGroupContext;

class FileSystemContext : public QObject
{
    Q_OBJECT
public:
    enum ContextFlag {
        fcNone = 0x0,
        fcGroup = 0x1,
        fcActive = 0x2,
        fcFileMod = 0x4,
        fcEditMod = 0x8,
        fcMissing = 0x10,
    };
    typedef QFlags<ContextFlag> ContextFlags;


    virtual ~FileSystemContext();

    int id() const;
    bool isGist() const;

    virtual const QString name();
    void setName(const QString& name);
    const QString& location() const;
    virtual void setLocation(const QString& location);

    const ContextFlags &flags();
    virtual void setFlag(ContextFlag flag);
    virtual void unsetFlag(ContextFlag flag);

    bool matches(const QString& name, bool isGist) const;
    FileGroupContext* parentEntry() const;
    void setParentEntry(FileGroupContext *parent);
    virtual FileSystemContext* childEntry(int index);
    virtual int childCount();

signals:
    void nameChanged(int id, QString newName);

protected:
    FileSystemContext(FileGroupContext* parent, int id, QString name, QString location, bool isGist);

protected:
    int mId;
    FileGroupContext* mParent;
    QString mName;
    QString mLocation;
    bool mIsGist;
    ContextFlags mFlags;

};

} // namespace ide
} // namespace gams

#endif // FILESYSTEMCONTEXT_H
