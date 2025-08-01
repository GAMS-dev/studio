#include "reportdialog.h"
#include "ui_reportdialog.h"
#include "../src/commonpaths.h"
#include <QFileInfo>

namespace gams {
namespace studio {

const int cLogLinesCheck = 10;
const QString cPathFindStudio("]: Started: ");
const QString cPathFindSettings("Settings path: ");
const int cDetailsHeight = 500;

ReportDialog::ReportDialog(DialogKind kind, QString details)
    : QDialog(nullptr)
    , ui(new Ui::Dialog)
{
    setWindowFlags(windowFlags() & ~Qt::MSWindowsFixedSizeDialogHint);
    ui->setupUi(this);

    mTempDir = new QTemporaryDir();
    setKind(kind);
    addReference(tiEmail, qgetenv("STUDIO_REPORT_EMAIL"));
    mOtherSize = QSize(-1, -1);
    QString mail = qgetenv("GAMS_STUDIO_EMAIL_CONTACT");
    if (!mail.isEmpty()) {
        ui->edMail->setText(mail);
        ui->cbMail->setChecked(true);
    }
    if (extractLog(tiLog, CommonPaths::studioDocumentsDir() + "/studio.log"))
        mLogIndex = tiLog;
    if (extractLog(tiLogBk, CommonPaths::studioDocumentsDir() + "/studio.log~") && mLogIndex < 0)
        mLogIndex = tiLogBk;

    ui->detailsText->setText(details);
    ui->detailsText->setVisible(false);
    ui->detailsText->setMinimumHeight(cDetailsHeight);

}

ReportDialog::~ReportDialog()
{
    delete mTempDir;
    delete ui;
}

void ReportDialog::setKind(DialogKind kind)
{
    if (kind == dkUnresponsive) {
        setWindowTitle("GAMS Studio is busy");
        ui->label->setText("GAMS Studio is unresponsive. If you want to wait a bit longer press Retry.");
    } else {
        setWindowTitle("GAMS Studio crashed");
        ui->label->setText("The application terminated unexpectedly.");
    }

    // TODO use
    QString advice = "You can help us to find the reason by sending a report.";

    ui->bRetry->setVisible(kind == dkUnresponsive);
}

void ReportDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (mOtherSize.height() < 0) {
        mOtherSize = geometry().size();
        mOtherSize.setHeight(mOtherSize.height() + (ui->detailsText->isVisible() ? -cDetailsHeight : cDetailsHeight));
        qDebug() << "stored height: " << mOtherSize.height();
    }
}

void ReportDialog::on_bClose_clicked()
{
    accept();
    close();
}

void ReportDialog::on_bDetails_clicked()
{
    ui->detailsText->setVisible(!ui->detailsText->isVisible());
    resize(mOtherSize);
    mOtherSize.setHeight(mOtherSize.height() + (ui->detailsText->isVisible() ? -cDetailsHeight : cDetailsHeight));
    adjustSize();
}

void ReportDialog::addReference(int index, const QString data)
{
    if (mReferences.length() < index - tiCount)
        qDebug() << "Warning: Many auto generated elements in references";

    while (mReferences.length() < index)
        mReferences << QString();

    if (mReferences.length() >= index) {
        if (mReferences.length() > index)
            mReferences.replace(index, data);
        else
            mReferences << data;
        return;
    }
}

bool ReportDialog::extractLog(TypedIndex index, QString file)
{
    QFile log = QFile(file);
    int i = 0;
    if (log.exists()) {
        addReference(index, file);
        if (log.open(QFile::ReadOnly)) {
            while (!log.atEnd() && i < cLogLinesCheck) {
                QString line = log.readLine();
                if (qsizetype p = line.indexOf(cPathFindStudio))
                    mStudioPath = line.mid(p + cPathFindStudio.length());

                if (line.startsWith(cPathFindSettings))
                    mSettingsPath = line.mid(cPathFindSettings.length());

                if (!mStudioPath.isEmpty() && mSettingsPath.isEmpty())
                    return true;
                ++i;
            }
            log.close();
        }
    }
    return false;
}



} // namespace studio
} // namespace gams
