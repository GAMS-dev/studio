#include "projectoptions.h"
#include "ui_projectoptions.h"
#include "file/pexgroupnode.h"
#include "file/filemeta.h"
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
    ui->edBaseDir->setMinimumWidth(fontMetrics().height()*30);
    ui->edBaseDir->setToolTip("Base directory: used as base folder to represent the files");
    ui->edWorkDir->setToolTip("Working directory: used as working directory to run GAMS");
    ui->laBaseDir->setToolTip(ui->edBaseDir->toolTip());
    ui->laWorkDir->setToolTip(ui->edWorkDir->toolTip());
    ui->bBaseDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    ui->bWorkDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    adjustSize();
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
        ui->edWorkDir->setText(QDir::toNativeSeparators(mProject->workDir()));
        ui->edBaseDir->setText(QDir::toNativeSeparators(mProject->location()));
        ui->edMainGms->setText(QDir::toNativeSeparators(mProject->runnableGms()->location()));
    }
    show();
}

void ProjectOptions::accept()
{
    if (ui->edName->text().trimmed().compare(mProject->name()))
        mProject->setName(ui->edName->text().trimmed());
    QString path = QDir::fromNativeSeparators(ui->edBaseDir->text()).trimmed();
    if (path.compare(mProject->location(), fsCaseSensitive()))
        mProject->setLocation(path);
    path = QDir::fromNativeSeparators(ui->edWorkDir->text()).trimmed();
    if (path.compare(mProject->workDir(), fsCaseSensitive()))
        mProject->setWorkDir(path);
    QDialog::accept();
}


void ProjectOptions::on_edWorkDir_textEdited(const QString &text)
{
    updateEditColor(ui->edWorkDir, text);
}

void ProjectOptions::on_edBaseDir_textEdited(const QString &text)
{
    updateEditColor(ui->edBaseDir, text);
}

void ProjectOptions::updateEditColor(QLineEdit *edit, const QString &text)
{
    QDir dir(text.trimmed());
    if (!dir.exists()) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        QPalette pal = edit->palette();
        pal.setColor(QPalette::Text, Theme::color(Theme::Mark_errorFg));
        edit->setPalette(pal);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        edit->setPalette(QPalette());
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
            if (dir.exists()) lineEdit->setText(QDir::toNativeSeparators(dir.path()));
        }
    });
    connect(dialog, &QFileDialog::finished, this, [dialog]() { dialog->deleteLater(); });
    dialog->setModal(true);
    dialog->open();
}

} // namespace project
} // namespace studio
} // namespace gams
