#include "pathselect.h"
#include "ui_pathselect.h"
#include "theme.h"
#include "commonpaths.h"

#include <QFileDialog>

namespace gams {
namespace studio {
namespace pathselect {

PathSelect::PathSelect(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PathSelect)
{
    ui->setupUi(this);
    ui->edWorkDir->setToolTip("Working directory: used as working directory to run GAMS");
    ui->bWorkDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    ui->edWorkDir->setText(CommonPaths::defaultWorkingDir());
}

PathSelect::~PathSelect()
{
    delete ui;
}

void PathSelect::on_edWorkDir_textChanged(const QString &text)
{
    QDir dir(text.trimmed());
    ui->pbOk->setEnabled(!text.trimmed().isEmpty());
    if (ui->pbOk->isEnabled() && !dir.exists()) {
        QPalette pal = qApp->palette();
        pal.setColor(QPalette::Text, Theme::color(Theme::Mark_errorFg));
        ui->edWorkDir->setPalette(pal);
        setWindowTitle("Create Working Directory");
        ui->pbOk->setText("Create");
    } else {
        ui->edWorkDir->setPalette(qApp->palette());
        setWindowTitle("Select Working Directory");
        ui->pbOk->setText("Ok");
    }
    ui->label->setText("");
}

void PathSelect::on_bWorkDir_clicked()
{
    showDirDialog("Select Working Directory", ui->edWorkDir, ui->edWorkDir->text().trimmed());
}

void PathSelect::showDirDialog(const QString &title, QLineEdit *lineEdit, const QString &defaultDir)
{
    QString path = QDir::fromNativeSeparators(lineEdit->text()).trimmed();
    QDir dir(path);
    if (path.isEmpty() || !dir.exists())
        path = defaultDir;
    QFileDialog *dialog = new QFileDialog(this, title, path);
    dialog->setFileMode(QFileDialog::Directory);
    connect(dialog, &QFileDialog::accepted, this, [this, lineEdit, dialog]() {
        if (dialog->selectedFiles().count() == 1) {
            QDir dir(dialog->selectedFiles().at(0).trimmed());
            lineEdit->setText(QDir::toNativeSeparators(dir.path()));
            emit workDirSelected(dir.path());
            accept();
        }
    });
    connect(dialog, &QFileDialog::finished, this, [dialog]() { dialog->deleteLater(); });
    dialog->setModal(true);
    dialog->open();
}

void PathSelect::on_pbOk_clicked()
{
    QString path(ui->edWorkDir->text());
    if (path.isEmpty()) return;
    QDir dir(path);
    if (!dir.mkpath(".")) {
        ui->label->setText("Creating Working Directory failed!");
        ui->pbOk->setEnabled(false);
    } else {
        emit workDirSelected(path);
        accept();
    }
}

void PathSelect::on_pbCancel_clicked()
{
    reject();
}



} // namespace pathselect
} // namespace studio
} // namespace gams
