#include "pathrequest.h"
#include "ui_pathrequest.h"
#include "projectrepo.h"
#include <QFileDialog>

namespace gams {
namespace studio {
namespace path {

PathRequest::PathRequest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PathRequest)
{
    ui->setupUi(this);
    setModal(true);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setWindowTitle("Select projects base directory");
    mInitialText = ui->laText->text();
    ui->bCheck->setEnabled(false);
}

PathRequest::~PathRequest()
{
    delete ui;
}

void PathRequest::init(ProjectRepo *repo, const QString &baseDir, const QVariantList &data)
{
    mData = data;
    mProjectRepo = repo;
    mInitalBasePath = baseDir;
    ui->edBaseDir->setText(baseDir);
}

bool PathRequest::checkProject()
{
    int count;
    int ignored;
    QStringList missed;
    int ok = mProjectRepo->checkRead(mData, count, ignored, missed, baseDir());
    if (ok) {
        if (ignored)
            ui->laText->setText(QString("All referenced necessary files found, %1 ignored.")
                                .arg(QString::number(ignored) + (ignored==1 ? " file" : "files")));
        else
            ui->laText->setText(QString("All referenced files found."));
        ui->laText->setToolTip(QString());
    } else {
        bool one = (missed.size() == 1);
        ui->laText->setText(mInitialText.arg(one ? "One file" : QString::number(missed.size()) + " files"));
        ui->laText->setToolTip("Missing file"+QString(missed.size()==1 ? "":"s")+":\n" + missed.join("\n"));
    }
    return missed.isEmpty();
}

QString PathRequest::baseDir() const
{
    return ui->edBaseDir->text().trimmed();
}

void PathRequest::on_bDir_clicked()
{
    showDirDialog("Select Base Directory", ui->edBaseDir);
}

void PathRequest::showDirDialog(const QString &title, QLineEdit *lineEdit)
{
    QFileDialog *dialog = new QFileDialog(this, title, ui->edBaseDir->text().trimmed());
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

void PathRequest::on_bCheck_clicked()
{
    checkProject();
    ui->bCheck->setEnabled(false);
}

void PathRequest::on_edBaseDir_textChanged(const QString &text)
{
    Q_UNUSED(text)
    ui->bCheck->setEnabled(true);
}


} // namespace path
} // namespace studio
} // namespace gams
