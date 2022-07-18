#include "efieditor.h"
#include "ui_efieditor.h"
#include <QFile>
#include <QTextStream>

namespace gams {
namespace studio {
namespace efi {

EfiEditor::EfiEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EfiEditor)
{
    ui->setupUi(this);
    ui->fsWidget->setShowProtection(true);
    connect(ui->fsWidget, &fs::FileSystemWidget::createClicked, this, &EfiEditor::writeFile);
}

EfiEditor::~EfiEditor()
{
    delete ui;
}

void EfiEditor::setWorkingDir(const QString &workDir)
{
    ui->fsWidget->setWorkingDirectory(workDir);
}

void EfiEditor::load(const QString &fileName)
{
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
        ui->fsWidget->setInfo(entries.isEmpty() ? "'%1' is empty" : "'%1' is loaded", true);
        ui->fsWidget->setSelectedFiles(entries);
        file.close();
    } else {
        if (file.exists())
            ui->fsWidget->setInfo("Can't load '%1'", false);
        else
            ui->fsWidget->setInfo("'%1' doesn't exist", true);
    }
}

void EfiEditor::writeFile()
{
    QFile file(mFileName);
    if (file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        for (const QString &line : ui->fsWidget->selectedFiles()) {
            file.write(line.toUtf8());
            file.write("\n");
        }
        file.close();
    }
}

} // namespace efi
} // namespace studio
} // namespace gams
