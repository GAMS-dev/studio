/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "filecontext.h"
#include "filegroupcontext.h"
#include "exception.h"

namespace gams {
namespace studio {

const QStringList FileContext::mDefaulsCodecs = QStringList() << "Utf-8" << "GB2312" << "Shift-JIS"
                                                              << "System" << "Windows-1250" << "Latin-1";

FileContext::FileContext(FileGroupContext *parent, int id, QString name, QString location)
    : FileSystemContext(parent, id, name, location, FileSystemContext::File)
{
    mCrudState = location.isEmpty() ? CrudState::eCreate : CrudState::eRead;
}

void FileContext::setCrudState(CrudState state)
{
    mCrudState = state;
    emit crudChanged(state);
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
        setCrudState(CrudState::eRead);
    }
}

void FileContext::load(QString codecName)
{
    if (!document())
        FATAL() << "There is no document assigned to the file " << location();

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
        FATAL() << "File can't be set to an empty location";
    // TODO(JM) adapt parent group
    // TODO (JM): handling if the file already exists
    FileSystemContext::setLocation(location);
    setCrudState(CrudState::eCreate);
}

QIcon FileContext::icon()
{
    QFileInfo fi(mLocation);
    if (QString(".gms.inc.txt.").indexOf(QString(".%1.").arg(fi.suffix()), 0, Qt::CaseInsensitive) >= 0)
        return QIcon(":/img/gams-w");
    return QIcon(":/img/file-alt");
}

void FileContext::setDocument(QTextDocument* doc)
{
    if (mDocument && doc)
        throw FATAL() << "document of " << location() << " cannot be replaced";
    mDocument = doc;
    // don't overwrite ContextState::eMissing
    if (mDocument)
        setFlag(FileSystemContext::cfActive);
    else {
        unsetFlag(FileSystemContext::cfActive);
        setCrudState(CrudState::eRead);
    }
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

const QString FileContext::caption()
{
    return mName + (mCrudState==CrudState::eUpdate ? "*" : "");
}

void FileContext::textChanged()
{
    if (mCrudState != CrudState::eUpdate) {
        setCrudState(CrudState::eUpdate);
        emit changed(mId);
    }
}

} // namespace studio
} // namespace gams
