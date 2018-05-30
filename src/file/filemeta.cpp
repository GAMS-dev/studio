#include "filemeta.h"
#include "filetype.h"
#include <QFileInfo>
#include <QPlainTextDocumentLayout>

namespace gams {
namespace studio {

FileMeta::FileMeta(QString location) : mLocation(location), mData(Data(location))
{
    mName = QFileInfo(mLocation).fileName();
    mDocument = new QTextDocument(this);
    mDocument->setDocumentLayout(new QPlainTextDocumentLayout(mDocument));
    mDocument->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
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

FileMeta::Data::Data(QString location)
{
    if (location.startsWith('[') && location.endsWith(']')) {
        type = &FileType::from(location.mid(1,location.length()-2));
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
