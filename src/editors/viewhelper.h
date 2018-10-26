#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include "common.h"
#include "codeedit.h"
#include "processlogedit.h"
#include "pagingtextview.h"
#include "gdxviewer/gdxviewer.h"
#include "lxiviewer/lxiviewer.h"
#include "reference/referenceviewer.h"
#include <QWidget>

namespace gams {
namespace studio {

class ViewHelper
{
    ViewHelper();
public:
    static FileId fileId(QWidget* widget);
    static void setFileId(QWidget* widget, FileId id);

    static NodeId groupId(QWidget* widget);
    static void setGroupId(QWidget* widget, NodeId id);

    static QString location(QWidget* widget);
    static void setLocation(QWidget* widget, QString location);

    inline static AbstractEdit* initEditorType(AbstractEdit* w, EditorType type) {
        if(w) w->setProperty("EditorType", int(type));
        return w;
    }
    inline static CodeEdit* initEditorType(CodeEdit* w) {
        if(w) w->setProperty("EditorType", int(EditorType::source));
        return w;
    }
    inline static ProcessLogEdit* initEditorType(ProcessLogEdit* w) {
        if(w) w->setProperty("EditorType", int(EditorType::log));
        return w;
    }
    inline static PagingTextView* initEditorType(PagingTextView* w) {
        if(w) w->setProperty("EditorType", int(EditorType::txtRo));
        return w;
    }
    inline static gdxviewer::GdxViewer* initEditorType(gdxviewer::GdxViewer* w) {
        if(w) w->setProperty("EditorType", int(EditorType::gdx));
        return w;
    }
    inline static lxiviewer::LxiViewer* initEditorType(lxiviewer::LxiViewer* w) {
        if(w) w->setProperty("EditorType", int(EditorType::lxiLst));
        return w;
    }
    inline static reference::ReferenceViewer* initEditorType(reference::ReferenceViewer* w) {
        if(w) w->setProperty("EditorType", int(EditorType::ref));
        return w;
    }

    inline static EditorType editorType(QWidget* w) {
        QVariant v = w ? w->property("EditorType") : QVariant();
        return (v.isValid() ? static_cast<EditorType>(v.toInt()) : EditorType::undefined);
    }

    inline static AbstractEdit* toAbstractEdit(QWidget* w) {
        EditorType t = editorType(w);
        if (t == EditorType::lxiLst)
            return toLxiViewer(w)->codeEdit();
        return (t == EditorType::log || t == EditorType::source || t == EditorType::txt)
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
    inline static PagingTextView* toTextView(QWidget* w) {
        return (editorType(w) == EditorType::txtRo) ? static_cast<PagingTextView*>(w) : nullptr;
    }
    inline static gdxviewer::GdxViewer* toGdxViewer(QWidget* w) {
        return (editorType(w) == EditorType::gdx) ? static_cast<gdxviewer::GdxViewer*>(w) : nullptr;
    }
    inline static lxiviewer::LxiViewer* toLxiViewer(QWidget* w) {
        return (editorType(w) == EditorType::lxiLst) ? static_cast<lxiviewer::LxiViewer*>(w) : nullptr;
    }
    inline static reference::ReferenceViewer* toReferenceViewer(QWidget* w) {
        return (editorType(w) == EditorType::ref) ? static_cast<reference::ReferenceViewer*>(w) : nullptr;
    }


};

} // namespace studio
} // namespace gams

#endif // VIEWHELPER_H
