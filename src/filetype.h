#ifndef FILETYPE_H
#define FILETYPE_H

#include <QtCore>

namespace gams {
namespace studio {

///
/// The FileType class defines all kind of file types and additional data and description. The instances are accessed
/// via static functions. On the first usage, the list is initialized.
///
class FileType
{
public:
    enum Kind {
        None,
        Gsp,
        Gms,
        Lst,
        Lxi,
        Gdx,
    };

public:
    Kind kind() const;
    QStringList suffix() const;
    QString description() const;
    bool autoReload() const;
    Kind dependant() const;

    bool operator ==(const FileType& fileType) const;
    bool operator !=(const FileType& fileType) const;
    bool operator ==(const FileType::Kind& kind) const;
    bool operator !=(const FileType::Kind& kind) const;

    static const QList<FileType*> list();
    static FileType& from(QString suffix);
    static FileType& from(Kind kind);


private:
    friend class FileRepository;

    static void clear();
    FileType(Kind kind, QString suffix, QString description, bool autoReload, const Kind dependant = None);

    const Kind mKind;
    const QStringList mSuffix;
    const QString mDescription;
    const bool mAutoReload;
    const Kind mDependant;

    static QList<FileType*> mList;
    static FileType* mNone;
};

} // namespace studio
} // namespace gams

#endif // FILETYPE_H
