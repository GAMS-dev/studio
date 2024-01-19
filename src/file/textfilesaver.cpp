#include "textfilesaver.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "logger.h"

namespace gams {
namespace studio {

TextFileSaver::TextFileSaver(QObject *parent)
    : QObject{parent}
{}

TextFileSaver::~TextFileSaver()
{
    if (!mFileName.isEmpty())
        DEB() << "File hasn't been closed properly: " << mFileName;
    if (!mTempFile.fileName().isEmpty())
        DEB() << "File hasn't been closed properly: " << mTempFile.fileName();
}

bool TextFileSaver::open(const QString &filename, const QString &tempMarker)
{
    if (filename.isEmpty()) {
        SysLogLocator::systemLog()->append("Open for writing: missing file name", LogMsgType::Error);
        return false;
    }
    // Find a temp-file name
    QString tempFileBase = filename + tempMarker;
    QString tempFileName = tempFileBase;
    int counter = 0;
    while (QFile::exists(tempFileName)) {
        ++counter;
        if (counter > 9) {
            SysLogLocator::systemLog()->append("Could not create file, please cleanup temporary file names of kind " + tempFileBase + "%", LogMsgType::Error);
            return false;
        }
        tempFileName = tempFileBase + QString::number(counter);
    }
    if (!mTempFile.fileName().isEmpty() &&  mTempFile.isOpen()) {
        DEB() << mTempFile.fileName() << " needs to be closed before being used again";
        return false;
    }

    mFileName = filename;
    mTempFile.setFileName(tempFileName);
    bool res = mTempFile.open(QFile::WriteOnly | QFile::Text);
    if (!res)
        SysLogLocator::systemLog()->append("Could not write-open file " + filename, LogMsgType::Error);
    return res;
}

qint64 TextFileSaver::write(const QByteArray &content)
{
    return mTempFile.write(content);
}

qint64 TextFileSaver::write(const char *content, qint64 len)
{
    return mTempFile.write(content, len);
}

qint64 TextFileSaver::write(const char *content)
{
    return mTempFile.write(content);
}

bool TextFileSaver::close()
{
    mTempFile.close();
    bool res = true;
    if (QFile::exists(mFileName))
        res = QFile::remove(mFileName);
    if (!res)
        SysLogLocator::systemLog()->append("Could not overwrite file " + mFileName, LogMsgType::Error);
    else {
        res = QFile::rename(mTempFile.fileName(), mFileName);
        if (!res)
            SysLogLocator::systemLog()->append("Could not rename temporary file '" + mTempFile.fileName() + "' to '" + mFileName + "'", LogMsgType::Error);
        else {
            mTempFile.setFileName("");
            mFileName = "";
        }
    }
    return res;
}

} // namespace studio
} // namespace gams
