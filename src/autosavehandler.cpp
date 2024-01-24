/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include "autosavehandler.h"
#include "mainwindow.h"
#include "commonpaths.h"
#include "logger.h"

#include <QMessageBox>
#include <QDir>
#include <QTextStream>

namespace gams {
namespace studio {

AutosaveHandler::AutosaveHandler(MainWindow *mainWindow)
    : mMainWindow(mainWindow)
{

}

QStringList AutosaveHandler::checkForAutosaveFiles(const QStringList &list)
{
    QStringList filters { "*.gms", "*.txt" };
    QStringList autsaveFiles;

    for (const auto &file : list) {
        QFileInfo fi(file);
        QString path = fi.absolutePath();
        if (!path.isEmpty()) {
            QDir dir(path);
            dir.setNameFilters(filters);
            QStringList files = dir.entryList(filters);
            for (auto file : std::as_const(files)) {
                if (file.startsWith(mAutosavedFileMarker)) {
                    QString autosaveFilePath = path+"/"+file;
                    file.replace(mAutosavedFileMarker, "");
                    QString originalFilePath = path+"/"+file;
                    QFileInfo origin(originalFilePath);
                    if (origin.exists())
                        autsaveFiles << autosaveFilePath;
                    else
                        QFile::remove(autosaveFilePath);
                }
            }
        }
    }
    autsaveFiles.removeDuplicates();
    return autsaveFiles;
}

void AutosaveHandler::recoverAutosaveFiles(const QStringList &autosaveFiles)
{
    if (autosaveFiles.isEmpty()) return;
    QString fileText = (autosaveFiles.size() == 1) ? "\""+autosaveFiles.first()+"\" was"
                                                   : QString::number(autosaveFiles.size())+" files were";
    fileText.replace(mAutosavedFileMarker,"");
    QMessageBox::StandardButton decision = QMessageBox::question(mMainWindow,
                                                                 "Recover autosave files",
                                                                 "Studio has shut down unexpectedly.\n"
                                                                 +fileText+" not saved correctly.\nDo you "
                                                                 "want to recover your last modifications?");
    if (decision == QMessageBox::Yes) {
        for (const auto& autosaveFile : autosaveFiles) {
            QString originalversion = autosaveFile;
            originalversion.replace(mAutosavedFileMarker, "");
            QFile destFile(originalversion);
            QFile srcFile(autosaveFile);
            mMainWindow->openFilePath(destFile.fileName(), nullptr, ogFindGroup);
            if (srcFile.open(QFile::ReadOnly)) {
                if (destFile.open(QFile::WriteOnly)) {
                    QTextStream in(&srcFile);
                    QString line = in.readAll() ;
                    QWidget* editor = mMainWindow->recent()->editor();
                    PExFileNode* fc = mMainWindow->projectRepo()->findFileNode(editor);
                    QTextCursor curs(fc->document());
                    curs.select(QTextCursor::Document);
                    curs.insertText(line);
                    destFile.close();
                    AbstractEdit *abstractEdit = dynamic_cast<AbstractEdit*>(editor);
                    if (editor)
                        abstractEdit->moveCursor(QTextCursor::Start);
                }
                srcFile.close();
            }
        }
    } else {
        for (const auto& file : autosaveFiles)
            QFile::remove(file);
    }
}

void AutosaveHandler::saveChangedFiles()
{
    for (auto editor : mMainWindow->constOpenEditors()) {
        PExFileNode* node = mMainWindow->projectRepo()->findFileNode(editor);
        if (!node) continue; // skips unassigned widgets like the welcome-page
        QString filepath = QFileInfo(node->location()).path();
        QString tempFile = filepath+"/"+mTempFileMarker+node->name();
        QString autosaveFile = filepath+"/"+mAutosavedFileMarker+node->name();
        if (node->isModified() && (node->file()->kind() == FileKind::Gms || node->file()->kind() == FileKind::Txt)) {
            QFile file(tempFile);
            file.open(QFile::WriteOnly);
            QTextStream out(&file);
            out << node->document()->toPlainText();
            out.flush();
            file.close();
            file.rename(autosaveFile);
        } else if (QFileInfo::exists(autosaveFile)) {
            QFile::remove(autosaveFile);
        }
        if (QFileInfo::exists(tempFile))
            QFile::remove(tempFile);
    }
}

void AutosaveHandler::clearAutosaveFiles(const QStringList &openTabs)
{
    const QStringList files = checkForAutosaveFiles(openTabs);
    for (const auto& file : files)
        QFile::remove(file);
}


} // namespace studio
} // namespace gams
