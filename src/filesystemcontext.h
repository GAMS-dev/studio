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
#ifndef FILESYSTEMCONTEXT_H
#define FILESYSTEMCONTEXT_H

#include <QtWidgets>
#include "codeeditor.h"
#include "logeditor.h"
#include "gdxviewer/gdxviewer.h"

namespace gams {
namespace studio {

typedef int FileId;

class FileGroupContext;

class FileSystemContext : public QObject
{   // TODO(AF) Make this thing abstract and use is as a interface for all common functions?
    // TODO(JM) Disagree: to many common members - this would lead to code doubling. If you want an interface,
    //                    it could be set on top of this FileSystemContext (e.g. AbstractContext)
    Q_OBJECT

public:
    enum ContextFlag { // TODO(AF) for global methods (e.g. save all) add changed state?
                       // TODO(JM) I'd prefer having either a pointer-list of changed context in repo or a method here

        // TODO(JM) check, which flags are better implemented as methods getting their info implicit
        cfNone          = 0x00,
        cfActive        = 0x01, // TODO(JM) implemented and in use: if this is the only real flag, we should have a method instead
        cfFileMod       = 0x02, // TODO(JM) implemented but not in use (marks changes from outside)
        cfEditMod       = 0x04, // TODO(JM) implemented but not in use (marks changes from inside - but here we have the doc.modified())
        cfMissing       = 0x08, // TODO(JM) some implementation missing in FileContext?
        cfExtendCaption = 0x10, // needed for doubled groups - could be moved to a boolean there
        cfVirtual       = 0x20, // set - but not used
    };

    enum ContextType {
        File,
        FileAction, // TODO(AF) still required?
                    // TODO(JM) I commonly use it - maybe we kill it after Welcome-Page has a quick start?
        FileGroup,
        FileSystem,
        Log
    };

    enum EditorType {
        etUndefined = 0,
        etPlainText = 1,
        etSourceCode = 2,
        etLastTextType = 4,

        etLxiLst = 5,
        etGdx = 6,
        etLastKomplexType = 9,
    };

    typedef QFlags<ContextFlag> ContextFlags;

    virtual ~FileSystemContext();

    FileId id() const;

    /// \brief File context type.
    /// \return Returns the file context type as <c>int</c>.
    int type() const;

    /// \brief Checks if the node can be represented in a tab.
    /// \return True, if the node can be represented in a tab.
    bool canShowAsTab() const;

    /// The caption of this file, which is its extended display name.
    /// \return The caption of this node.
    virtual const QString caption();

    /// The raw name of this node.
    /// \return The raw name of this node.
    virtual const QString name();

    /// Sets the raw name of this node.
    /// \param name The raw name of this node.
    void setName(const QString& name);

    /// The location of the node. This is a directory or file with full path.
    /// \param location The new location
    const QString& location() const;

    /// Sets a new location (name and path) to the node. This sets the CRUD-state to "Create"
    /// \param location The new location
    virtual void setLocation(const QString& location);

    /// The icon for this file type.
    /// \return The icon for this file type.
    virtual QIcon icon() = 0;

    const ContextFlags &flags() const;
    virtual void setFlag(ContextFlag flag, bool value = true);
    virtual void unsetFlag(ContextFlag flag);
    virtual bool testFlag(ContextFlag flag);

    FileGroupContext* parentEntry() const;
    virtual void setParentEntry(FileGroupContext *parent);
    virtual FileSystemContext* childEntry(int index) const;
    virtual int childCount() const;

    FileSystemContext *findFile(QString filePath);


public: // static convenience methods
    inline static void initEditorType(CodeEditor* w) {
        if(w) w->setProperty("EditorType", etSourceCode);
    }
    inline static void initEditorType(QPlainTextEdit* w) { // obsolete?
        if(w) w->setProperty("EditorType", etPlainText);
    }
    inline static void initEditorType(LogEditor* w) {
        if(w) w->setProperty("EditorType", etPlainText);
    }
    inline static void initEditorType(gdxviewer::GdxViewer* w) {
        if(w) w->setProperty("EditorType", etGdx);
    }
    inline static int editorType(QWidget* w) {
        QVariant v = w ? w->property("EditorType") : QVariant();
        return (v.isValid() ? v.toInt() : etUndefined);
    }
    inline static QPlainTextEdit* toPlainEdit(QWidget* w) {
        int t = editorType(w);
        return (t > etUndefined && t <= etLastTextType) ? static_cast<QPlainTextEdit*>(w) : nullptr;
    }
    inline static CodeEditor* toCodeEdit(QWidget* w) {
        return (editorType(w) == etSourceCode) ? static_cast<CodeEditor*>(w) : nullptr;
    }
    inline static gdxviewer::GdxViewer* toGdxViewer(QWidget* w) {
        return (editorType(w) == etGdx) ? static_cast<gdxviewer::GdxViewer*>(w) : nullptr;
    }

signals:
    void changed(FileId fileId);

protected:
    friend class LogContext;

    FileSystemContext(FileId fileId, QString name, QString location);
    FileSystemContext(FileId fileId, QString name, QString location, ContextType type);
    virtual void checkFlags();

private:
    FileId mId;
    FileGroupContext* mParent;
    QString mName;
    QString mLocation;
    ContextFlags mFlags;
    ContextType mType;
};

} // namespace studio
} // namespace gams

#endif // FILESYSTEMCONTEXT_H
