/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include "common.h"
#include "connect/connecteditor.h"
#include "editors/codeedit.h"
#include "editors/textview.h"
#include "file/projectedit.h"
#include "gdxviewer/gdxviewer.h"
#include "lxiviewer/lxiviewer.h"
#include "option/gamsconfigeditor.h"
#include "option/solveroptionwidget.h"
#include "reference/referenceviewer.h"
#include "engine/efieditor.h"
#include <QWidget>

namespace gams {
namespace studio {

#ifdef _WIN32
    static const QString allFilesFilter = "*.*";
#else
    // using *.* would exclude all files without extension
    static const QString allFilesFilter = "*";
#endif

class ViewHelper
{
    ViewHelper();
public:
    static QString location(QWidget* widget);
    static void setLocation(QWidget* widget, const QString &location);

    static bool modified(const QWidget *widget);
    static void setModified(QWidget* widget, bool modified);

    inline static project::ProjectEdit* initEditorType(project::ProjectEdit* w) {
        if(w) w->setProperty("EditorType", int(EditorType::pro));
        return w;
    }
    inline static AbstractEdit* initEditorType(AbstractEdit* w, EditorType type) {
        if(w) w->setProperty("EditorType", int(type));
        return w;
    }
    inline static CodeEdit* initEditorType(CodeEdit* w) {
        if(w) w->setProperty("EditorType", int(EditorType::source));
        return w;
    }
    inline static TextView* initEditorType(TextView* w, EditorType type) {
        Q_ASSERT(type == EditorType::txtRo || type == EditorType::lxiLstChild || type == EditorType::log);
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
    inline static connect::ConnectEditor* initEditorType(connect::ConnectEditor * w) {
        if(w) w->setProperty("EditorType", int(EditorType::gConYaml));
        return w;
    }
    inline static option::GamsConfigEditor* initEditorType(option::GamsConfigEditor * w) {
        if(w) w->setProperty("EditorType", int(EditorType::gucfg));
        return w;
    }
    inline static efi::EfiEditor* initEditorType(efi::EfiEditor * w) {
        if(w) w->setProperty("EditorType", int(EditorType::efi));
        return w;
    }

    inline static EditorType editorType(QWidget* w) {
        if (!w) return EditorType::undefined;
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
        if (t == EditorType::txtRo || t == EditorType::lxiLstChild || t == EditorType::log)
            return static_cast<TextView*>(w);
        EditorType pt = editorType(w->parentWidget());
        if (pt == EditorType::txtRo || t == EditorType::lxiLstChild || pt == EditorType::log)
            return static_cast<TextView*>(w->parentWidget());
        return nullptr;
    }
    inline static AbstractView* toAbstractView(QWidget* w) {
        return (editorType(w) >= EditorType::gdx) ? static_cast<AbstractView*>(w) : nullptr;
    }
    inline static gdxviewer::GdxViewer* toGdxViewer(QWidget* w) {
        return (editorType(w) == EditorType::gdx) ? static_cast<gdxviewer::GdxViewer*>(w) : nullptr;
    }
    inline static lxiviewer::LxiViewer* toLxiViewer(QWidget* w) {
        if (editorType(w) == EditorType::lxiLstChild && w && w->parentWidget() && w->parentWidget()->parentWidget())
            return static_cast<lxiviewer::LxiViewer*>(w->parentWidget()->parentWidget());
        if (editorType(w) == EditorType::lxiLst)
            return static_cast<lxiviewer::LxiViewer*>(w);
        QWidget *par = w;
        while (par && ViewHelper::editorType(par) != EditorType::lxiLst)
            par = par->parentWidget();
        return (editorType(par) == EditorType::lxiLst) ? static_cast<lxiviewer::LxiViewer*>(par) : nullptr;
    }
    inline static project::ProjectEdit* toProjectEdit(QWidget* w) {
        return (editorType(w) == EditorType::pro) ? static_cast<project::ProjectEdit*>(w) : nullptr;
    }
    inline static reference::ReferenceViewer* toReferenceViewer(QWidget* w) {
        return (editorType(w) == EditorType::ref) ? static_cast<reference::ReferenceViewer*>(w) : nullptr;
    }
    inline static option::SolverOptionWidget* toSolverOptionEdit(QWidget* w) {
        return (editorType(w) == EditorType::opt) ? static_cast<option::SolverOptionWidget*>(w) : nullptr;
    }
    inline static connect::ConnectEditor* toGamsConnectEditor(QWidget* w) {
        return (editorType(w) == EditorType::gConYaml) ? static_cast<connect::ConnectEditor*>(w) : nullptr;
    }
    inline static option::GamsConfigEditor* toGamsConfigEditor(QWidget* w) {
        return (editorType(w) == EditorType::gucfg) ? static_cast<option::GamsConfigEditor*>(w) : nullptr;
    }
    inline static efi::EfiEditor* toEfiEditor(QWidget* w) {
        return (editorType(w) == EditorType::efi) ? static_cast<efi::EfiEditor*>(w) : nullptr;
    }

    static QStringList dialogProjectFilter() {
        QStringList res("GAMS Studio Project (*.gsp)");
        return res;
    }

    static const QString dialogGdxFilter() {
        QString res("GDX file (*.gdx);;");
        res.append("All files (" + allFilesFilter + ")");
        return res;
    }

    static const QString dialogOptFileFilter(const QString &solverName) {
        QString res("%1 option file (%1.opt %1.*);;All files (" + allFilesFilter + ")");
        res = res.arg(solverName);
        return res;
    }

    static const QString dialogPfFileFilter() {
        QString res("Gams Parameter File (*.pf);;");
        res.append("All files (" + allFilesFilter + ")");
        return res;
    }

    static const QString dialogSettingsFileFilter() {
        QString res("GAMS user settings (*.gus);;");
        res.append("All files (" + allFilesFilter + ")");
        return res;
    }

    static QStringList dialogFileFilterUserCreated() {
        QStringList res("GAMS source (*.gms)");
        QStringList userTypes = FileType::userGamsTypes();
        if (!userTypes.isEmpty())
            res << ("Additional GAMS sources (*." + userTypes.join(" *.") + ")");
        res << "GAMS project (*.gsp)";
        res << "GAMS Include files (*.inc)"
            << "Option files (*.opt *.op* *.o*)"
            << "GAMS Configuration file (gamsconfig.yaml)"
            << "GAMS Connect YAML file (*.yaml)"
            << "Parameter file (*.pf)"
            << "Text files (*.txt)"
            << "External files (*.efi)"
            << "All files (" + allFilesFilter + ")";
        return res;
    }

    static QStringList dialogFileFilterAll(bool addProject = false) {
        QStringList res(addProject ? "GAMS source or project (*.gms *.gsp)" : "GAMS source (*.gms)");
        QStringList userTypes = FileType::userGamsTypes();
        QString allGams(QString("All GAMS files (*.gms%1 *.gdx *.inc *.log *.lst *efi *.opt *.op* *.o *.pf *.ref *.dmp%2)")
                        .arg(addProject ? " *.gsp" : ""));
        if (!userTypes.isEmpty()) {
            res << allGams.arg(" *." + userTypes.join(" *."));
            res << ("Additional GAMS sources (*." + userTypes.join(" *.") + ")");
        } else
            res << allGams.arg("");
        res << "GAMS Include files (*.inc)"
            << "Option files (*.opt *.op* *.o*)"
            << "GAMS Configuration file (gamsconfig.yaml)"
            << "GAMS Connect YAML file (*.yaml)"
            << "GAMS Data eXchange (*.gdx)"
            << "Reference files (*.ref)"
            << "Parameter file (*.pf)"
            << "Text files (*.txt)"
            << "External files (*.efi)"
            << "All files (" + allFilesFilter + ")";
        return res;
    }

    static bool updateBaseTheme();

    static void setAppearance(int appearance = -1);
    static void changeAppearance(int appearance = -1);
};

} // namespace studio
} // namespace gams

#endif // VIEWHELPER_H
