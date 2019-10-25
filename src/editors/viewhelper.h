/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "codeedit.h"
#include "processlogedit.h"
#include "textview.h"
#include "gdxviewer/gdxviewer.h"
#include "lxiviewer/lxiviewer.h"
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
    inline static ProcessLogEdit* initEditorType(ProcessLogEdit* w) {
        if(w) w->setProperty("EditorType", int(EditorType::log));
        return w;
    }
    inline static TextView* initEditorType(TextView* w) {
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
    inline static option::SolverOptionWidget* initEditorType(option::SolverOptionWidget * w) {
        if(w) w->setProperty("EditorType", int(EditorType::opt));
        return w;
    }

    inline static EditorType editorType(QWidget* w) {
        QVariant v = w ? w->property("EditorType") : QVariant();
        return (v.isValid() ? static_cast<EditorType>(v.toInt()) : EditorType::undefined);
    }

    inline static AbstractEdit* toAbstractEdit(QWidget* w) {
        EditorType t = editorType(w);
        return (t == EditorType::log || t == EditorType::syslog || t == EditorType::source || t == EditorType::txt)
                ? static_cast<AbstractEdit*>(w) : nullptr;
    }
    inline static CodeEdit* toCodeEdit(QWidget* w) {
        EditorType t = editorType(w);
        return (t == EditorType::source || t == EditorType::txt) ? static_cast<CodeEdit*>(w) : nullptr;
    }
    inline static ProcessLogEdit* toLogEdit(QWidget* w) {
        return (editorType(w) == EditorType::log) ? static_cast<ProcessLogEdit*>(w) : nullptr;
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
    inline static option::SolverOptionWidget* toSolverOptionEdit(QWidget* w) {
        return (editorType(w) == EditorType::opt) ? static_cast<option::SolverOptionWidget*>(w) : nullptr;
    }


};

} // namespace studio
} // namespace gams

#endif // VIEWHELPER_H
