#include "projectoptions.h"
#include "ui_projectoptions.h"
#include "file/pexgroupnode.h"
#include "theme.h"

#include <QDir>
#include <QPushButton>
#include <QFont>
#include <QFileDialog>

namespace gams {
namespace studio {
namespace project {

Qt::CaseSensitivity fsCaseSensitive()
{
#ifdef __unix__
    return Qt::CaseSensitive;
#else
    return Qt::CaseInsensitive;
#endif
}

ProjectOptions::ProjectOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectOptions)
{
    ui->setupUi(this);
    ui->edMainGms->setEnabled(false);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    setModal(true);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
}

ProjectOptions::~ProjectOptions()
{
    delete ui;
}

void ProjectOptions::showProject(PExProjectNode *project)
{
    if (!project) return;
    mProject = project;
    if (mProject) {
        ui->edName->setText(mProject->name());
        ui->edWorkDir->setText(mProject->workDir());
        ui->edBaseDir->setText(mProject->location());
        ui->edMainGms->setText(mProject->mainModelName());
    }
    show();
}

void ProjectOptions::accept()
{
    if (ui->edName->text().trimmed().compare(mProject->name()))
        mProject->setName(ui->edName->text().trimmed());
    if (ui->edWorkDir->text().trimmed().compare(mProject->workDir(), fsCaseSensitive()))
        mProject->setWorkDir(ui->edWorkDir->text().trimmed());
    QDialog::accept();
}


void ProjectOptions::on_edWorkDir_textEdited(const QString &text)
{
    QDir dir(text.trimmed());
    if (!dir.exists()) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        QPalette pal = ui->edWorkDir->palette();
        pal.setColor(QPalette::Text, Theme::color(Theme::Mark_errorFg));
        ui->edWorkDir->setPalette(pal);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        ui->edWorkDir->setPalette(QPalette());
    }
}

void ProjectOptions::on_bWorkDir_clicked()
{
    showDirDialog("Select Working Directory", ui->edWorkDir);
}

void ProjectOptions::on_bBaseDir_clicked()
{
    showDirDialog("Select Base Directory", ui->edBaseDir);
}

void ProjectOptions::showDirDialog(const QString &title, QLineEdit *lineEdit)
{
    QFileDialog *dialog = new QFileDialog(this, title, mProject->location());
    dialog->setFileMode(QFileDialog::Directory);
    connect(dialog, &QFileDialog::accepted, this, [lineEdit, dialog]() {
        if (dialog->selectedFiles().count() == 1) {
            QDir dir(dialog->selectedFiles().first().trimmed());
            if (dir.exists()) lineEdit->setText(dir.path());
        }
    });
    connect(dialog, &QFileDialog::finished, this, [dialog]() { dialog->deleteLater(); });
    dialog->setModal(true);
    dialog->show();
}

} // namespace project
} // namespace studio
} // namespace gams
