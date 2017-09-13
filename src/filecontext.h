#ifndef FILECONTEXT_H
#define FILECONTEXT_H

#include <QtCore>
#include <QtWidgets>

// TODO(JM) This is a stubb file that needs to be reimplemented or at least reviewed

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

enum class UpdateScope {
    all,
    storage,
    editor,
    name,
    state
};

class FileContext : public QObject
{
    Q_OBJECT
public:
    int id();
    QString name() const;
    const QFileInfo& fileInfo() const;
    void rename(QString newFilePath);

    QString codec() const;
    void setCodec(const QString& codec);

signals:
    void fileInfoChanged(int id, QString newFilePath);
    void nameChangedById(int id, QString newName);
    void nameChangedByIdStr(const QString &identString, const QString &newName);
    void pushName(const QString &newName);

public slots:
    void textChanged();

private:
    friend class FileRepository;
    FileContext(int id, QString fileName = QString());

private:
    const int mId;
    QString mName;
    QFileInfo mFileInfo;
    FileType mFileType = FileType::ftGms;
    CrudState mCrudState;
    bool mActive = false;
    QString mCodec = "UTF-8";
};


} // namespace ide
} // namespace gams

#endif // FILECONTEXT_H
