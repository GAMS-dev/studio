#include "filecontext.h"
#include <QTextStream>
#include <QDebug>

namespace gams {
namespace ide {

FileContext::FileContext(int id, QString fileName): mId(id), mFileInfo(QFileInfo(fileName))
{
    QString suffix = mFileInfo.suffix();
    QString pattern = ".gms";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftGms;
    pattern = ".txt.text";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftTxt;
    pattern = ".inc";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftInc;
    pattern = ".log";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftLog;
    pattern = ".lst";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftLst;
    pattern = ".lxi";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftLxi;
}

QString FileContext::codec() const
{
    return mCodec;
}

void FileContext::setCodec(const QString& codec)
{
    // TODO(JM) changing the codec must trigger conversion (not necessarily HERE)
    mCodec = codec;
}

void FileContext::textChanged()
{
    qDebug() << "Text changed";
    if (mCrudState != CrudState::eUpdate) {
        mCrudState = CrudState::eUpdate;
        emit nameChanged(mId, name());
        emit pushName(name());
        qDebug() << "FIRST Text changed";
    }
}
int FileContext::id()
{
    return mId;
}

QString FileContext::name()
{
    return mFileInfo.baseName() + (mCrudState==CrudState::eUpdate ? "*" : "");
}

const QFileInfo &FileContext::fileInfo()
{
    return mFileInfo;
}

void FileContext::rename(QString newFilePath)
{
    QString oldName = name();
    QString oldFilePath = mFileInfo.filePath();
    if (mFileInfo.filePath().isEmpty()) {
        mFileInfo.setFile(newFilePath);
    } else {
        // TODO(JM) this has to be handled carefully (name-change, file-rename, allow also move file?)
        qDebug() << "renaming of already assigned file not implemented";
    }
    if (oldFilePath != mFileInfo.filePath())
        emit fileInfoChanged(mId, mFileInfo.filePath());
    if (oldName != name())
        emit nameChanged(mId, name());
}

} // namespace ide
} // namespace gams
