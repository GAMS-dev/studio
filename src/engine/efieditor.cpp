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
#include "efieditor.h"
#include "ui_efieditor.h"
#include "file/textfilesaver.h"
#include <QFile>
#include <QTextStream>

namespace gams {
namespace studio {
namespace efi {

EfiEditor::EfiEditor(QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::EfiEditor)
{
    ui->setupUi(this);
    ui->fsWidget->setShowProtection(true);
    ui->fsWidget->setCreateVisible(false);
    connect(ui->fsWidget, &fs::FileSystemWidget::createClicked, this, &EfiEditor::requestSave);
    connect(ui->fsWidget, &fs::FileSystemWidget::modified, this, [this]() { setModified(true); });
    mModified = false;
}

EfiEditor::~EfiEditor()
{
    delete ui;
}

void EfiEditor::setWorkingDir(const QString &workDir)
{
    ui->fsWidget->setWorkingDirectory(workDir);
    updateInfoText(mFileName.isEmpty() ? " - no filename assigned" : "", !mFileName.isEmpty());
}

void EfiEditor::setModelName(const QString &name)
{
    ui->fsWidget->setModelName(name);
}

void EfiEditor::load(const QString &fileName)
{
    if (ui->fsWidget->workingDirectory().isEmpty())
        return;
    mFileName = fileName;
    QFile file(fileName);
    QStringList entries;
    if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream textStream(&file);
        while (true) {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            if (!line.trimmed().isEmpty())
                entries << line;
        }
        ui->fsWidget->setSelectedFiles(entries);
        file.close();
        updateInfoText("", true);
    } else {
        updateInfoText(QString(file.exists() ? "- Can't load '%1'" : "- '%1' doesn't exist").arg(fileName), file.exists());
    }
    mModified = false;
    setModified(false);
}

bool EfiEditor::isModified()
{
    return mModified;
}

void EfiEditor::setWarnText(const QString &text)
{
    updateInfoText(text, false);
}

void EfiEditor::selectFilter()
{
    ui->fsWidget->selectFilter();
}

bool EfiEditor::save(const QString &fileName)
{
    TextFileSaver file;
    if (file.open(fileName)) {
        const QStringList lines = ui->fsWidget->selectedFiles();
        for (const QString &line : lines) {
            file.write(line.toUtf8());
            file.write("\n");
        }
        if (file.close()) {
            mFileName = fileName;
            ui->fsWidget->clearMissingFiles();
            setModified(false);
            return true;
        }
    }
    return false;
}

void EfiEditor::updateInfoText(const QString &extraText, bool valid)
{
    if (ui->fsWidget->workingDirectory().isEmpty())
        ui->fsWidget->setInfo("No working directory", false);
    else
        ui->fsWidget->setInfo(QString("%1   %2").arg(ui->fsWidget->workingDirectory(), extraText), valid);
}

void EfiEditor::setModified(bool modified)
{
    if (!ui->fsWidget->missingFiles().isEmpty()) modified = true;
    if (modified == mModified) return;
    mModified = modified;
    emit modificationChanged(mModified);
}

} // namespace efi
} // namespace studio
} // namespace gams
