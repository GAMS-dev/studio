/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
 */
#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include "common.h"
#include "editors/codeedit.h"
#include "editors/processlogedit.h"
#include "editors/textview.h"
#include "gdxviewer/gdxviewer.h"
#include "lxiviewer/lxiviewer.h"
#include "option/gamsconfigeditor.h"
#include "option/solveroptionwidget.h"
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
    inline static option::SolverOptionWidget* initEditorType(option::SolverOptionWidget * w) {
        if(w) w->setProperty("EditorType", int(EditorType::opt));
        return w;
    }
    inline static option::GamsConfigEditor* initEditorType(option::GamsConfigEditor * w) {
        if(w) w->setProperty("EditorType", int(EditorType::gcfg));
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
    inline static TextView* toTextView(QWidget* w) {
        if (!w) return nullptr;
        EditorType t = editorType(w);
        if (t == EditorType::lxiLst)
            return toLxiViewer(w)->textView();
        if (t == EditorType::txtRo || t == EditorType::log)
            return static_cast<TextView*>(w);
        EditorType pt = editorType(w->parentWidget());
        if (pt == EditorType::txtRo || pt == EditorType::log)
            return static_cast<TextView*>(w->parentWidget());
        return nullptr;
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
    inline static option::SolverOptionWidget* toSolverOptionEdit(QWidget* w) {
        return (editorType(w) == EditorType::opt) ? static_cast<option::SolverOptionWidget*>(w) : nullptr;
    }
    inline static option::GamsConfigEditor* toGamsConfigEditor(QWidget* w) {
        return (editorType(w) == EditorType::gcfg) ? static_cast<option::GamsConfigEditor*>(w) : nullptr;
    }

    inline static QStringList dialogFileFilterUserCreated() {
        return QStringList("GAMS source (*.gms)")
               << "Text files (*.txt)"
               << "Option files (*.opt *.op* *.o*)"
               << "Gams Config file (*.yaml)"
               << "All files (*.*)";
    }

    inline static QStringList dialogFileFilterAll() {
        return QStringList("GAMS Source (*.gms)")
               << "All GAMS Files (*.gms *.gdx *.log *.lst *.opt *.op* *.o *.ref *.dmp)"
               << "Option files (*.opt *.op* *.o*)"
               << "Gams Config file (*.yaml)"
               << "Reference files (*.ref)"
               << "Text files (*.txt)"
               << "All files (*.*)";
    }
    static void setAppearance(int appearance = -1);
    static void changeAppearance(int appearance);

};

} // namespace studio
} // namespace gams

#endif // VIEWHELPER_H
