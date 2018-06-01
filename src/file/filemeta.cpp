#include "filemeta.h"
#include "filetype.h"
#include <QFileInfo>
#include <QPlainTextDocumentLayout>
#include <QTextCodec>

namespace gams {
namespace studio {

FileMeta::FileMeta(FileId id, QString location) : mId(id), mLocation(location), mData(Data(location))
{
    bool symbolic = mLocation.startsWith('[');
    int braceEnd = mLocation.indexOf(']');
    if (braceEnd <= 0) braceEnd = mLocation.size();
    mName = symbolic ? mLocation.left(braceEnd) : QFileInfo(mLocation).fileName();
    mDocument = new QTextDocument(this);
    mDocument->setDocumentLayout(new QPlainTextDocumentLayout(mDocument));
    mDocument->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

inline FileId FileMeta::id() const
{
    return mId;
}

QString FileMeta::location() const
{
    return mLocation;
}

FileKind FileMeta::kind()
{
    return mData.type->kind();
}

QString FileMeta::name()
{
    return mName;
}

bool FileMeta::isModified() const
{
    return mDocument->isModified();
}

QTextDocument *FileMeta::document() const
{
    return mDocument;
}

int FileMeta::codecMib() const
{
    return mCodec ? mCodec->mibEnum() : -1;
}

bool FileMeta::exists() const
{
    return mData.exist;
}

FileMeta::Data::Data(QString location)
{
    if (location.startsWith('[')) {
        int len = location.indexOf(']')-2;
        type = (len > 0) ? &FileType::from(location.mid(1, len)) : &FileType::from("");
    } else {
        QFileInfo fi(location);
        exist = fi.exists();
        size = fi.size();
        created = fi.created();
        modified = fi.lastModified();
        type = &FileType::from(fi.suffix());
    }
}

} // namespace studio
} // namespace gams
