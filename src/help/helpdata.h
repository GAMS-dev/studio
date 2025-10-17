/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#ifndef HELPLOCATION_H
#define HELPLOCATION_H

#include <QList>
#include <QString>
#include <QPair>
#include <QUrl>

#include "commonpaths.h"
#include "support/solverconfiginfo.h"

namespace gams {
namespace studio {
namespace help {

enum struct DocumentType {
    Main,
    ReleaseNotes,
    DollarControl,
    GamsCall,
    Options,
    SolversMain,
    Solvers,
    ToolsMain,
    Tools,
    APIsMain,
    APIs,
    Index,
    StudioMain,
    Connect,
    ModLibs
};
enum struct StudioSection {
    WelcomePage,
    ProjectExplorer,
    ListingViewer,
    GDXDiff,
    GDXViewer,
    SearchAndReplace,
    ReferenceFileViewer,
    SolverOptionEditor,
    GamsUserConfigEditor,
    Toolbar,
    OptionEditor,
    ProcessLog,
    ParameterFile,
    ConnectEditor,
    EFIEditor,
};

class HelpData
{
    HelpData() {}

public:
// since C++17
//    inline static const QString START_CHAPTER = docs + "/index.html";
//    inline static const QString LATEST_ONLINE_HELP_URL = "https://www.gams.com/latest";

    static const QList<QPair<DocumentType, QString>> chapterLocation() {
        auto docs = CommonPaths::documentationDir();
        QList<QPair<DocumentType, QString>> list = {
            {DocumentType::Main, docs + "/index.html"},
            {DocumentType::ReleaseNotes, docs + "/RN_MAIN.html"},
            {DocumentType::DollarControl, docs + "/UG_DollarControlOptions.html"},
            {DocumentType::GamsCall, docs + "/UG_GamsCall.html"},
            {DocumentType::Options, docs + "/UG_OptionStatement.html"},
            {DocumentType::SolversMain, docs + "/S_MAIN.html"},
            {DocumentType::APIsMain, docs + "/API_MAIN.html"},
            {DocumentType::ToolsMain, docs + "/T_MAIN.html"},
            {DocumentType::Index, docs + "/keyword.html"},
            {DocumentType::StudioMain, docs + "/T_STUDIO.html"},
            {DocumentType::Connect, docs + "/UG_GAMSCONNECT.html"}
        };
        return list;
    }
    static const QList<QPair<StudioSection, QString>> studioSectionName() {
        QList<QPair<StudioSection, QString>> list = {
            {StudioSection::WelcomePage, "Welcome Page"},
            {StudioSection::ProjectExplorer, "Project Explorer"}, // Project Explorer or STUDIO_PROJECT_EXPLORER?
            {StudioSection::ListingViewer, "Listing Viewer"},
            {StudioSection::GDXDiff, "GDXDiff"},
            {StudioSection::GDXViewer, "GDX Viewer"},
            {StudioSection::SearchAndReplace, "Search and Replace"},
            {StudioSection::ReferenceFileViewer, "Reference File Viewer"},
            {StudioSection::SolverOptionEditor, "Solver Option Editor"},
            {StudioSection::GamsUserConfigEditor, "GAMS Configuration Editor"},
            {StudioSection::OptionEditor, "Option Editor"},
            {StudioSection::ProcessLog, "Process Log"},
            {StudioSection::ParameterFile, "Parameter File"},
            {StudioSection::ConnectEditor, "Connect Editor"},
            {StudioSection::EFIEditor, "GAMS Engine"},
        };
        return list;
    }
    static const QList<QPair<QString, QString>> solverChaperLocation() {
        auto docs = CommonPaths::documentationDir();
        QList<QPair<QString, QString>> list;
        QMap<QString, QString> exceptionlist = {
            {"conopt", "conopt4"},
            {"decisc", "decis"},
            {"decism", "decis"},
            {"ipopth", "ipopt"},
            {"lindoglobal", "lindo"},
        };

        support::SolverConfigInfo solverInfo;
        auto solverNames = solverInfo.solverNames().values();
        for (const QString& s : std::as_const(solverNames)) {
            if (exceptionlist.contains(s.toLower())) {
                list << QPair<QString,QString>(s.toLower(), docs + "/S_" + exceptionlist.value(s.toLower()).toUpper()+".html");
            } else {
                list << QPair<QString,QString>(s.toLower(), docs + "/S_" + s.toUpper()+".html");
            }
        }
        return list;
    }

    inline static QUrl getLatestOnlineHelpUrl() {
        return QUrl("https://www.gams.com/latest/", QUrl::TolerantMode);
    }

    inline static QString getChapterLocation(const DocumentType type) {
        QString location;
        for (const QPair<DocumentType, QString> &list: chapterLocation()) {
            if (list.first == DocumentType::Main)
                location = list.second;
            if (list.first == type)
                return list.second;
        }
        return location;
    }

    inline static QString getStudioSectionName(const StudioSection section) {
        QString name;
        for (const QPair<StudioSection, QString> &list: studioSectionName()) {
            if (list.first == section)
                return list.second;
        }
        return name;
    }

    inline static QString getGamsCallOptionAnchor(const QString &keyword) {
        if (keyword.isEmpty())
            return keyword;
        else
            return QString("%1%2").arg("GAMSAO", keyword.toLower());
    }

    inline static QString getDollarControlOptionAnchor(const QString &keyword) {
        if (keyword.isEmpty()) return keyword;

        if (keyword.startsWith("off", Qt::CaseInsensitive))
            return QString("%1%2").arg("DOLLARon", keyword.toLower());
        else if (keyword.startsWith("on", Qt::CaseInsensitive))
               return QString("%1%2").arg("DOLLARonoff", keyword.toLower().mid(2));
        else
           return QString("%1%2").arg("DOLLAR", keyword.toLower());
    }

    inline static QString getKeywordIndexAnchor(const QString &keyword) {
        return QString("q=%1").arg(keyword.toLower());
    }

    inline static QString getSolverOptionAnchor(const QString &solvername, const QString &keyword) {
        if (keyword.isEmpty()) return keyword;

        QString str = keyword;
        str.replace(" ", "_");
        str.replace("/", "_");
        str.replace(".", "DOT");
        QMap<QString, QString> solverlist = {
            {"conopt4", "conopt"},
        };
        if (solverlist.contains(solvername.toLower()))
            return QString("%1%2").arg( solverlist[solvername.toLower()].toUpper(), str);
        else
            return QString("%1%2").arg( solvername.toUpper(), str);
    }

    inline static QString getSolverChapterLocation(const QString &solverName) {
        for (const QPair<QString, QString> &list: solverChaperLocation()) {
            if (QString::compare(solverName, list.first, Qt::CaseInsensitive) ==0)
                return list.second;
        }
        return getChapterLocation(DocumentType::Solvers);
    }

    inline static QString getStudioSectionAnchor(const QString &section) {
        if (section.isEmpty()) return section;

        QString str = section.toUpper().simplified();
        str.replace(" ", "_");
        return QString("STUDIO_%1").arg(str);
    }

    inline static QString getConnectAnchor(const QString &agent, const QString &attribute) {
        if (agent.isEmpty()) return agent;

        return (attribute.isEmpty() ? QString("%1").arg(agent.toUpper())
                                    : QString("%1_%2").arg(agent.toUpper(), attribute.toUpper()) );
    }

    inline static int getURLIndexFrom(const QString &urlStr)  {
        int index = -1;
        for(const QString &path : getPathList()) {
            index++;
            if (urlStr.lastIndexOf(path) > -1)
               return index;
        }
        return index;
    }

    inline static const QStringList getPathList() {
        QStringList pathList = {
            "/docs",
            "/gamslib_ml", "/testlib_ml", "/datalib_ml", "/emplib_ml",
            "/apilib_ml", "/finlib_ml", "/noalib_ml", "/psoptlib_ml"
        };
        return  pathList;
    }
};

}
}
}
#endif // HELPLOCATION_H
