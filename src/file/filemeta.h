#ifndef FILEMETA_H
#define FILEMETA_H

#include <QWidget>
#include <QDateTime>
#include <QTextDocument>
#include "syntax.h"
#include "editors/codeeditor.h"
#include "editors/logeditor.h"
#include "gdxviewer/gdxviewer.h"
#include "lxiviewer/lxiviewer.h"

namespace gams {
namespace studio {

class ProjectRunGroupNode;

class FileMeta: public QObject
{
    Q_OBJECT
public:
    inline FileId id() const;
    QString location() const;
    FileKind kind();
    QString name();
    bool isModified() const;
    QTextDocument* document() const;
    int codecMib() const;
    bool exists() const;
    bool isOpen() const;

    QWidget *createEdit(QTabWidget* tabWidget, bool focus, ProjectRunGroupNode *runGroup = nullptr, int codecMip = -1);
    QWidgetList editors() const;
    QWidget* topEditor() const;
    void addEditor(QWidget* edit);
    void editToTop(QWidget* edit);
    void removeEditor(QWidget* edit);
    void removeAllEditors();
    bool hasEditor(QWidget* edit);

    void jumpTo(const QTextCursor& cursor, FileId runId, bool focus, int altLine = 0, int altColumn = 0);

public: // static convenience methods
    inline static void initEditorType(CodeEditor* w) {
        if(w) w->setProperty("EditorType", (int)EditorType::source);
    }
    inline static void initEditorType(LogEditor* w) {
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

    inline static AbstractEditor* toAbstractEdit(QWidget* w) {
        EditorType t = editorType(w);
        if (t == EditorType::lxiLst)
            return toLxiViewer(w)->codeEditor();
        return (t == EditorType::log || t == EditorType::source)
                ? static_cast<AbstractEditor*>(w) : nullptr;
    }
    inline static CodeEditor* toCodeEdit(QWidget* w) {
        EditorType t = editorType(w);
        if (t == EditorType::lxiLst)
            return toLxiViewer(w)->codeEditor();
        return (t == EditorType::source) ? static_cast<CodeEditor*>(w) : nullptr;
    }
    inline static LogEditor* toLogEdit(QWidget* w) {
        return (editorType(w) == EditorType::log) ? static_cast<LogEditor*>(w) : nullptr;
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
    void updateMarks();

private:
    struct Data {
        Data(QString location);
        bool exist = false;
        qint64 size = 0;
        QDateTime created;
        QDateTime modified;
        FileType *type = nullptr;
    };

    friend class FileMetaRepo;
    FileMeta(FileMetaRepo* fileRepo, FileId id, QString location);

private:
    FileId mId;
    FileMetaRepo* mFileRepo;
    QString mLocation;
    QString mName;
    Data mData;
    QTextCodec *mCodec = nullptr;
    QTextDocument* mDocument = nullptr;
    QWidgetList mEditors;
    ErrorHighlighter* mHighlighter;

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
