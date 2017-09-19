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
    virtual const QString name();
    CrudState crudState() const;
    void setLocation(const QString &location); // equals save_as...

    virtual void setFlag(ContextFlag flag);
    virtual void unsetFlag(ContextFlag flag);

    void save();
    void load(QString codecName = QString());
    void setDocument(QTextDocument *doc);
    QTextDocument* document();

public slots:
    void textChanged();
//    void storageChanged();

protected:
    friend class FileRepository;
    FileContext(FileGroupContext *parent, int id, QString name, QString location, bool isGist);

private:
    CrudState mCrudState = CrudState::eCreate;
    QString mCodec = "UTF-8";
    QTextDocument* mDocument = nullptr;
    static const QStringList mDefaulsCodecs;

};

} // namespace ide
} // namespace gams

#endif // FILECONTEXT_H
