#include "filecontext.h"
#include "filegroupcontext.h"

namespace gams {
namespace ide {

const QStringList FileContext::mDefaulsCodecs = QStringList() << "Utf-8" << "GB2312" << "Shift-JIS"
                                                              << "System" << "Windows-1250" << "Latin-1";

FileContext::FileContext(FileGroupContext *parent, int id, QString name, QString location, bool isGist)
    : FileSystemContext(parent, id, name, location, isGist)
{
    mCrudState = location.isEmpty() ? CrudState::eCreate : CrudState::eRead;
}

CrudState FileContext::crudState() const
{
    return mCrudState;
}

void FileContext::save()
{
    if (mCrudState != CrudState::eRead) {
        if (location().isEmpty())
            throw QException();
        QFile file(location());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            throw QException();
        QTextStream out(&file);
        out.setCodec(mCodec.toLatin1().data());
        qDebug() << "Saving with Codec set to: "<< mCodec;
        out << mDocument->toPlainText();
        out.flush();
        file.close();
        mCrudState = CrudState::eRead;
        emit nameChanged(mId, name());
    }
}

void FileContext::load(QString codecName)
{
    if (!document())
        throw QException();

    QStringList codecNames = codecName.isEmpty() ? mDefaulsCodecs : QStringList() << codecName;
    QFile file(location());
    if (!file.fileName().isEmpty() && file.exists()) {
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            const QByteArray data(file.readAll());
            QString text;
            QString nameOfUsedCodec;
            for (QString tcName: codecNames) {
                QTextCodec::ConverterState state;
                QTextCodec *codec = QTextCodec::codecForName(tcName.toLatin1().data());
                if (codec) {
                    nameOfUsedCodec = tcName;
                    text = codec->toUnicode(data.constData(), data.size(), &state);
                    if (state.invalidChars == 0) {
                        qDebug() << "opened with codec " << nameOfUsedCodec;
                        break;
                    }
                    qDebug() << "Codec " << nameOfUsedCodec << " contains " << QString::number(state.invalidChars) << "invalid chars.";
                } else {
                    qDebug() << "System doesn't contain codec " << nameOfUsedCodec;
                    nameOfUsedCodec = QString();
                }
            }
            if (!nameOfUsedCodec.isEmpty()) {
                mDocument->setPlainText(text);
                mCodec = nameOfUsedCodec;
            }
            file.close();
        }
    }
}

void FileContext::setLocation(const QString& location)
{
    if (location.isEmpty())
        throw QException();  // context is already bound to a file
    // TODO(JM) adapt parent group
    FileSystemContext::setLocation(location);
    mCrudState = CrudState::eCreate;
}

void FileContext::setFlag(ContextFlag flag)
{
    if (flag == FileSystemContext::fcGroup)
        throw QException();
    FileSystemContext::setFlag(flag);
}

void FileContext::unsetFlag(ContextFlag flag)
{
    if (flag == FileSystemContext::fcGroup)
        throw QException();
    FileSystemContext::unsetFlag(flag);
}

void FileContext::setDocument(QTextDocument* doc)
{
    if (mDocument)
        throw QException();
    mDocument = doc;
    // don't overwrite ContextState::eMissing
    if (mDocument)
        setFlag(FileSystemContext::fcActive);
    else
        unsetFlag(FileSystemContext::fcActive);
}

QTextDocument*FileContext::document()
{
    return mDocument;
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

const QString FileContext::name()
{
    return mName + (mCrudState==CrudState::eUpdate ? "*" : "");
}

void FileContext::textChanged()
{
    if (mCrudState != CrudState::eUpdate) {
        mCrudState = CrudState::eUpdate;
        emit nameChanged(mId, name());
    }
}

} // namespace ide
} // namespace gams
