#include "autosavehandler.h"
#include "mainwindow.h"
#include "commonpaths.h"

#include <QMessageBox>
#include <QDir>
#include <QJsonObject>
#include <QTextStream>

namespace gams {
namespace studio {

AutosaveHandler::AutosaveHandler(MainWindow *mainWindow)
    : mMainWindow(mainWindow)
{

}

QStringList AutosaveHandler::checkForAutosaveFiles(QStringList list)
{
    QStringList filters { "*.gms", "*.txt" };
    QStringList autsaveFiles;

    for (auto file : list)
    {
        QFileInfo fi(file);
        QString path = fi.absolutePath();
        if (!path.isEmpty()) {
            QDir dir(path);
            dir.setNameFilters(filters);
            QStringList files = dir.entryList(filters);
            for (auto file : files) {
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
    int decision = QMessageBox::question(mMainWindow,
                                         "Recover autosave files",
                                         "Studio has shut down unexpectedly. Some"
                                         "files were not saved correctly. Do you "
                                         "want to recover your last modifications?",
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::Yes);

    if (QMessageBox::Yes == decision) {
        for (const auto& autosaveFile : autosaveFiles)
        {
            QString originalversion = autosaveFile;
            originalversion.replace(mAutosavedFileMarker, "");
            QFile destFile(originalversion);
            QFile srcFile(autosaveFile);
            mMainWindow->openFile(destFile.fileName());
            if (srcFile.open(QIODevice::ReadWrite))
            {
                if (destFile.open(QIODevice::ReadWrite))
                {
                    QTextStream in(&srcFile);
                    QString line = in.readAll() ;
                    QWidget* editor = mMainWindow->recent()->editor();
                    ProjectFileNode* fc = mMainWindow->projectRepo()->fileNode(editor);
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
    for (auto editor : mMainWindow->openEditors())
    {
        ProjectFileNode* fc = mMainWindow->projectRepo()->fileNode(editor);
        QString filepath = QFileInfo(fc->location()).path();
        QString filename = filepath+fc->name();
        QString autosaveFile = filepath+"/"+mAutosavedFileMarker+fc->name();
        if (fc->isModified() && (fc->metrics().fileType() == FileType::Gms || fc->metrics().fileType() == FileType::Txt))
        {
            QFile file(autosaveFile);
            file.open(QIODevice::WriteOnly);
            QTextStream out(&file);
            out << fc->document()->toPlainText();
            out.flush();
            file.close();
        }
        else if (QFileInfo::exists(autosaveFile)) {
                QFile::remove(autosaveFile);
        }
    }
}

void AutosaveHandler::clearAutosaveFiles(const QStringList &openTabs)
{
    for (const auto& file : checkForAutosaveFiles(openTabs))
        QFile::remove(file);
}


} // namespace studio
} // namespace gams
