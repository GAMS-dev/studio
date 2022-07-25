#include "efieditor.h"
#include "ui_efieditor.h"
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
    connect(ui->fsWidget, &fs::FileSystemWidget::selectionCountChanged, this, &EfiEditor::updateSelCount);
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
    if (text.isEmpty())
        updateSelCount();
    else
        updateInfoText(text, false);
}

void EfiEditor::selectFilter()
{
    ui->fsWidget->selectFilter();
}

void EfiEditor::updateSelCount()
{
    setModified(true);
    int count = ui->fsWidget->selectionCount();
    updateInfoText(QString(count ? "- selected %1 file%2" : "- no selection")
                   .arg(count).arg(count > 1 ? "s" : ""), true);

}

void EfiEditor::save(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        for (const QString &line : ui->fsWidget->selectedFiles()) {
            file.write(line.toUtf8());
            file.write("\n");
        }
        file.close();
        mFileName = fileName;
        ui->fsWidget->clearMissingFiles();
        setModified(false);
    }
}

void EfiEditor::updateInfoText(QString extraText, bool valid)
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
