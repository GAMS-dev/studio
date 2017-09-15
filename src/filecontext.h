#ifndef FILECONTEXT_H
#define FILECONTEXT_H

#include "filesystemcontext.h"

namespace gams {
namespace ide {

// TODO(JM) define extra type class that gathers all type info (enum, suffix, description, icon, ...)
enum class FileType {
    ftGms,
    ftTxt,
    ftInc,
    ftLog,
    ftLst,
    ftLxi,
};

enum class CrudState {
    eCreate,
    eRead,
    eUpdate,
    eDelete
};


class FileGroupContext;

class FileContext : public FileSystemContext
{
    Q_OBJECT
public:
    QString codec() const;
    void setCodec(const QString& codec);
    const QString name();

public slots:
    void textChanged();

protected:
    friend class FileRepository;
    FileContext(FileGroupContext *parent, int id, QString name, QString location, bool isGist);

private:
    CrudState mCrudState;
    QString mCodec = "UTF-8";

};

} // namespace ide
} // namespace gams

#endif // FILECONTEXT_H
