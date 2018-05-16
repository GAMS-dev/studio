/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "filesystemcontext.h"
#include "filegroupcontext.h"
#include "logger.h"

namespace gams {
namespace studio {

FileSystemContext::FileSystemContext(FileId fileId, QString name, QString location)
    : QObject(), mId(fileId), mParent(nullptr), mName(name), mLocation(location), mFlags(cfNone), mType(FileSystem)
{}

FileSystemContext::FileSystemContext(FileId fileId, QString name, QString location, ContextType type)
    : QObject(), mId(fileId), mParent(nullptr), mName(name), mLocation(location), mFlags(cfNone), mType(type)
{}

void FileSystemContext::checkFlags()
{
}

FileSystemContext::~FileSystemContext()
{
    if (mParent) {
        FileGroupContext* group = mParent;
        mParent = nullptr;
        if (group) group->removeChild(this);
    }
}

FileId FileSystemContext::id() const
{
    return mId;
}

int FileSystemContext::type() const
{
    return mType;
}

bool FileSystemContext::canShowAsTab() const
{
    static QList<int> showableTypes = {ContextType::File};
    return showableTypes.contains(mType);
}

FileGroupContext* FileSystemContext::parentEntry() const
{
    return mParent;
}

void FileSystemContext::setParentEntry(FileGroupContext* parent)
{
    if (parent != mParent) {
        if (mParent) mParent->removeChild(this);
        mParent = parent;
        if (mParent) mParent->insertChild(this);
    }
}

FileSystemContext* FileSystemContext::childEntry(int index) const
{
    Q_UNUSED(index);
    return nullptr;
}

int FileSystemContext::childCount() const
{
    return 0;
}

const QString FileSystemContext::caption()
{
    return mName;
}

const QString FileSystemContext::name()
{
    return mName;
}

void FileSystemContext::setName(const QString& name)
{
    if (mName != name) {
        mName = name;
        emit changed(mId);
    }
}

const QString& FileSystemContext::location() const
{
    return mLocation;
}

void FileSystemContext::setLocation(const QString& location)
{
    if (!location.isEmpty()) {
        QFileInfo fi(location);
        if(!fi.exists()) {
            QFile newFile(location);
            newFile.open(QIODevice::WriteOnly);
            newFile.close();
        }
        mLocation = fi.absoluteFilePath();
        setName(fi.fileName());
    }
}

const FileSystemContext::ContextFlags& FileSystemContext::flags() const
{
    return mFlags;
}

void FileSystemContext::setFlag(ContextFlag flag, bool value)
{
    bool current = testFlag(flag);
    if (current == value) return;
    mFlags.setFlag(flag, value);
    if (mParent)
        mParent->checkFlags();
    emit changed(mId);
}

void FileSystemContext::unsetFlag(ContextFlag flag)
{
    setFlag(flag, false);
}

bool FileSystemContext::testFlag(FileSystemContext::ContextFlag flag)
{
    return mFlags.testFlag(flag);
}

FileSystemContext* FileSystemContext::findFile(QString filePath)
{
    if(location() == filePath)
        return this;
    else
        return nullptr;
}

inline void FileSystemContext::initEditorType(CodeEditor *w) {
    if(w) w->setProperty("EditorType", etSourceCode);
}

inline void FileSystemContext::initEditorType(AbstractEditor *w) { // obsolete?
    if(w) w->setProperty("EditorType", etPlainText);
}

inline void FileSystemContext::initEditorType(LogEditor *w) {
    if(w) w->setProperty("EditorType", etLog);
}

inline void FileSystemContext::initEditorType(gdxviewer::GdxViewer *w) {
    if(w) w->setProperty("EditorType", etGdx);
}

inline void FileSystemContext::initEditorType(lxiviewer::LxiViewer *w) {
    if(w) w->setProperty("EditorType", etLxiLst);
}

inline int FileSystemContext::editorType(QWidget *w) {
    QVariant v = w ? w->property("EditorType") : QVariant();
    return (v.isValid() ? v.toInt() : etUndefined);
}

inline AbstractEditor *FileSystemContext::toAbstractEdit(QWidget *w) {
    int t = editorType(w);
    if (t == etLxiLst)
        return toLxiViewer(w)->codeEditor();
    return (t > etUndefined && t <= etLastTextType) ? static_cast<AbstractEditor*>(w) : nullptr;
}

inline CodeEditor *FileSystemContext::toCodeEdit(QWidget *w) {
    int t = editorType(w);
    if (t == etLxiLst)
        return toLxiViewer(w)->codeEditor();
    return (t == etSourceCode) ? static_cast<CodeEditor*>(w) : nullptr;
}

inline LogEditor *FileSystemContext::toLogEdit(QWidget *w) {
    return (editorType(w) == etLog) ? static_cast<LogEditor*>(w) : nullptr;
}

inline gdxviewer::GdxViewer *FileSystemContext::toGdxViewer(QWidget *w) {
    return (editorType(w) == etGdx) ? static_cast<gdxviewer::GdxViewer*>(w) : nullptr;
}

inline lxiviewer::LxiViewer *FileSystemContext::toLxiViewer(QWidget *w) {
    return (editorType(w) == etLxiLst) ? static_cast<lxiviewer::LxiViewer*>(w) : nullptr;
}

} // namespace studio
} // namespace gams
