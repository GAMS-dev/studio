#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include "common.h"
#include "codeedit.h"
#include "processlogedit.h"
#include "textview.h"
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
    inline static TextView* initEditorType(TextView* w, EditorType type) {
        Q_ASSERT(type == EditorType::txtRo || type == EditorType::lxiLst || type == EditorType::log);
        if(w) w->setProperty("EditorType", int(type));
        return w;
    }
//    inline static TextView* initEditorType(TextView* w) {
//        if(w) w->setProperty("EditorType", int(EditorType::txtRo));
//        return w;
//    }
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
        return (t == EditorType::syslog || t == EditorType::source || t == EditorType::txt)
                ? static_cast<AbstractEdit*>(w) : nullptr;
    }
    inline static CodeEdit* toCodeEdit(QWidget* w) {
        EditorType t = editorType(w);
        return (t == EditorType::source || t == EditorType::txt) ? static_cast<CodeEdit*>(w) : nullptr;
    }
    inline static TextView* toLogEdit(QWidget* w) {
        return (editorType(w) == EditorType::log) ? static_cast<TextView*>(w) : nullptr;
    }
    inline static TextView* toTextView(QWidget* w) {
        EditorType t = editorType(w);
        if (t == EditorType::lxiLst)
            return toLxiViewer(w)->textView();
        return (t == EditorType::txtRo) ? static_cast<TextView*>(w)
                                        : (w && editorType(w->parentWidget()) == EditorType::txtRo)
                                          ? static_cast<TextView*>(w->parentWidget())  : nullptr;
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
