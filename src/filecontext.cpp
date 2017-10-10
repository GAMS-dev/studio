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
}

CrudState FileContext::crudState() const
{
    return mCrudState;
}

void FileContext::save()
{
    if (document() && mCrudState != CrudState::eRead) {
        if (location().isEmpty())
            EXCEPT() << "Can't save without file name";
        QFile file(location());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            EXCEPT() << "Can't open the file";
        mMetrics = FileMetrics();
        QTextStream out(&file);
        out.setCodec(mCodec.toLatin1().data());
        qDebug() << "Saving with Codec set to: "<< mCodec;
        out << document()->toPlainText();
        out.flush();
        file.close();
        mMetrics = FileMetrics(QFileInfo(file));
        document()->setModified(false);
        setCrudState(CrudState::eRead);
    }
}

void FileContext::load(QString codecName)
{
    if (!document())
        EXCEPT() << "There is no document assigned to the file " << location();

    QStringList codecNames = codecName.isEmpty() ? mDefaulsCodecs : QStringList() << codecName;
    QFile file(location());
    if (!file.fileName().isEmpty() && file.exists()) {
        if (!file.open(QFile::ReadOnly | QFile::Text))
            EXCEPT() << "Error opening file " << location();
        mMetrics = FileMetrics();
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
            document()->setPlainText(text);
            mCodec = nameOfUsedCodec;
        }
        file.close();
        document()->setModified(false);
        mMetrics = FileMetrics(QFileInfo(file));
    }
    if (!mWatcher) {
        mWatcher = new QFileSystemWatcher(this);
        connect(mWatcher, &QFileSystemWatcher::fileChanged, this, &FileContext::onFileChangedExtern);
        qDebug() << "Watching " << location();
        mWatcher->addPath(location());
    }
}

const QList<QPlainTextEdit*> FileContext::editors() const
{
    return mEditors;
}

void FileContext::setLocation(const QString& location)
{
    if (location.isEmpty())
        EXCEPT() << "File can't be set to an empty location.";
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

void FileContext::addEditor(QPlainTextEdit* edit)
{
    if (!edit || mEditors.contains(edit))
        return;
    mEditors.append(edit);
    if (mEditors.size() == 1) {
        document()->setParent(this);
        connect(document(), &QTextDocument::modificationChanged, this, &FileContext::modificationChanged, Qt::UniqueConnection);
    } else {
        edit->setDocument(mEditors.first()->document());
    }
    setFlag(FileSystemContext::cfActive);
}

void FileContext::removeEditor(QPlainTextEdit* edit)
{
    int i = mEditors.indexOf(edit);
    if (i < 0)
        return;
    mEditors.removeAt(i);
    if (mEditors.isEmpty()) {
        // After removing last editor: paste document-parency back to editor
        edit->document()->setParent(edit);
        unsetFlag(FileSystemContext::cfActive);
        mCrudState = CrudState::eRead;
    }
}

void FileContext::removeAllEditors()
{
    while (!mEditors.isEmpty()) {
        removeEditor(mEditors.first());
    }
}

bool FileContext::hasEditor(QPlainTextEdit* edit)
{
    return mEditors.contains(edit);
}

QTextDocument*FileContext::document()
{
    if (mEditors.isEmpty())
        return nullptr;
    return mEditors.first()->document();
}

FileContext::~FileContext()
{
    removeAllEditors();
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

void FileContext::modificationChanged(bool modiState)
{
    // TODO(JM) check what todo on CrudState::eDelete
    if (modiState && mCrudState != CrudState::eUpdate) {
        setCrudState(CrudState::eUpdate);
        emit changed(mId);
        qDebug() << "modificationChanged to " << (modiState?"changed":"unchanged");
    }
    if (!modiState && mCrudState == CrudState::eUpdate) {
        setCrudState(CrudState::eRead);
        emit changed(mId);
        qDebug() << "modificationChanged to " << (modiState?"changed":"unchanged");
    }
}

void FileContext::onFileChangedExtern(QString filepath)
{
    qDebug() << "changed: " << filepath;
    QFileInfo fi(filepath);
    FileMetrics::ChangeKind changeKind = mMetrics.check(fi);
    if (changeKind == FileMetrics::ckSkip) return;
    if (changeKind == FileMetrics::ckUnchanged) return;
    if (!fi.exists()) {
        // file has been renamed or deleted
        if (document()) document()->setModified();
        emit deletedExtern(mId);
    }
    if (changeKind == FileMetrics::ckModified) {
        // file changed externally
        emit modifiedExtern(mId);
    }
}

} // namespace studio
} // namespace gams
