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
#ifndef FILEMETA_H
#define FILEMETA_H

#include <QWidget>
#include <QDateTime>
#include <QTextDocument>
#include "syntax.h"
#include "editors/codeedit.h"
#include "editors/processlogedit.h"
#include "gdxviewer/gdxviewer.h"
#include "lxiviewer/lxiviewer.h"

class QTabWidget;

namespace gams {
namespace studio {

class ProjectRunGroupNode;

class FileMeta: public QObject
{
    Q_OBJECT
public:
    ~FileMeta() override;
    FileId id() const;
    QString location() const;
    QStringList suffix() const;
    FileKind kind();
    QString name(NameModifier mod = NameModifier::raw);
    QTextDocument* document() const;
    int codecMib() const;
    void setCodecMib(int mib);
    QTextCodec *codec() const;
    void setCodec(QTextCodec *codec);
    bool exists() const;
    bool isOpen() const;
    bool isModified() const;
    bool isReadOnly() const;
    bool isAutoReload() const;

    QWidget *createEdit(QTabWidget* tabWidget, ProjectRunGroupNode *runGroup = nullptr, QList<int> codecMibs = QList<int>());
    QWidgetList editors() const;
    QWidget* topEditor() const;
    void addEditor(QWidget* edit);
    void editToTop(QWidget* edit);
    void removeEditor(QWidget* edit, bool suppressCloseSignal = false);
    bool hasEditor(QWidget * const &edit) const;
    void load(int codecMib);
    void load(QList<int> codecMibs = QList<int>());
    void save();
    void saveAs(const QString &location);
    void renameToBackup();

    void jumpTo(NodeId groupId, bool focus, int line = 0, int column = 0);
    void rehighlight(int line);
    void rehighlightBlock(QTextBlock block, QTextBlock endBlock = QTextBlock());
    ErrorHighlighter* highlighter() const;


public: // static convenience methods
    inline static void initEditorType(CodeEdit* w) {
        if(w) w->setProperty("EditorType", (int)EditorType::source);
    }
    inline static void initEditorType(ProcessLogEdit* w) {
        if(w) w->setProperty("EditorType", (int)EditorType::log);
    }
    inline static void initEditorType(gdxviewer::GdxViewer* w) {
        if(w) w->setProperty("EditorType", (int)EditorType::gdx);
    }
    inline static void initEditorType(lxiviewer::LxiViewer* w) {
        if(w) w->setProperty("EditorType", (int)EditorType::lxiLst);
    }

    inline static EditorType editorType(QWidget* w) {
        QVariant v = w ? w->property("EditorType") : QVariant();
        return (v.isValid() ? static_cast<EditorType>(v.toInt()) : EditorType::undefined);
    }

    inline static AbstractEdit* toAbstractEdit(QWidget* w) {
        EditorType t = editorType(w);
        if (t == EditorType::lxiLst)
            return toLxiViewer(w)->codeEdit();
        return (t == EditorType::log || t == EditorType::source)
                ? static_cast<AbstractEdit*>(w) : nullptr;
    }
    inline static CodeEdit* toCodeEdit(QWidget* w) {
        EditorType t = editorType(w);
        if (t == EditorType::lxiLst)
            return toLxiViewer(w)->codeEdit();
        return (t == EditorType::source) ? static_cast<CodeEdit*>(w) : nullptr;
    }
    inline static ProcessLogEdit* toLogEdit(QWidget* w) {
        return (editorType(w) == EditorType::log) ? static_cast<ProcessLogEdit*>(w) : nullptr;
    }
    inline static gdxviewer::GdxViewer* toGdxViewer(QWidget* w) {
        return (editorType(w) == EditorType::gdx) ? static_cast<gdxviewer::GdxViewer*>(w) : nullptr;
    }
    inline static lxiviewer::LxiViewer* toLxiViewer(QWidget* w) {
        return (editorType(w) == EditorType::lxiLst) ? static_cast<lxiviewer::LxiViewer*>(w) : nullptr;
    }

signals:
    void changed(FileId fileId);
    void documentOpened();
    void documentClosed();
    void openFile(FileMeta* fileMeta, bool focus = true, FileId runId = -1, int codecMib = -1);

private slots:
    void modificationChanged(bool modiState);

private:
    struct Data {
        Data(QString location, FileType *knownType = nullptr);
        bool exist = false;
        qint64 size = 0;
        QDateTime created;
        QDateTime modified;
        FileType *type = nullptr;
    };

    friend class FileMetaRepo;
    FileMeta(FileMetaRepo* fileRepo, FileId id, QString location, FileType *knownType = nullptr);
    QVector<QPoint> getEditPositions();
    void setEditPositions(QVector<QPoint> edPositions);
    void internalSave(const QString &location);

private:
    FileId mId;
    FileMetaRepo* mFileRepo;
    QString mLocation;
    QString mName;
    Data mData;
    QWidgetList mEditors;
    QTextCodec *mCodec = nullptr;
    QTextDocument* mDocument = nullptr;
    ErrorHighlighter* mHighlighter = nullptr;

    static const QList<int> mDefaulsCodecs;

    // TODO(JM): QTextBlock.userData  ->  TextMark
    // TODO(JM): TextChanged events
    // TODO(JM): FileChanged events
    // TODO(JM): Autosave
    // TODO(JM): Data-Reference ( QTextDocument / GDX / LST+LXI / ... )
    // TODO(JM): FileState (opened, closed, changed, removed, ...)
    // TODO(JM): FileType info

};

} // namespace studio
} // namespace gams

#endif // FILEMETA_H
