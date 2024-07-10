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
#include "gdxdiffdialog.h"
#include "ui_gdxdiffdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include "viewhelper.h"
#include "gdxviewer/gdxviewer.h"
#include "gdxdiffprocess.h"
#include "mainwindow.h"
#include "keys.h"

namespace gams {
namespace studio {
namespace gdxdiffdialog {

GdxDiffDialog::GdxDiffDialog(QWidget *parent) :
    QDialog(parent, Qt::Tool),
    ui(new Ui::GdxDiffDialog),
    mProc(new GdxDiffProcess(this))
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->lineEdit_4->setValidator(new QDoubleValidator());
    ui->lineEdit_5->setValidator(new QDoubleValidator());

    connect(mProc.get(), &GdxDiffProcess::finished, this, &GdxDiffDialog::diffDone);

    connect(ui->leDiff, &QLineEdit::textEdited, this, [this]() {mPrepopulateDiff = false;});
    connect(ui->leInput1, &QLineEdit::textChanged, this, [this]() {prepopulateDiff();});

    adjustSize();
    reset();
}

GdxDiffDialog::~GdxDiffDialog()
{
    cancelProcess(50);
    const QValidator *v = ui->lineEdit_4->validator();
    ui->lineEdit_4->setValidator(nullptr);
    delete v;
    v = ui->lineEdit_5->validator();
    ui->lineEdit_5->setValidator(nullptr);
    delete v;
    delete ui;
}

QStringList GdxDiffDialog::gdxDiffParamters()
{
    QStringList args;
    args << QDir::toNativeSeparators(mLastInput1);
    args << QDir::toNativeSeparators(mLastInput2);
    if (!mLastDiffFile.isEmpty())
        args << QDir::toNativeSeparators(mLastDiffFile);
    args << "Field=" + ui->cbFieldToCompare->itemText(ui->cbFieldToCompare->currentIndex());
    auto eps = ui->lineEdit_4->text().trimmed();
    if (!eps.isEmpty())
        args << "Eps=" + eps;
    auto relEps = ui->lineEdit_5->text().trimmed();
    if (!relEps.isEmpty())
        args << "RelEps=" + relEps;
    if (ui->cbFieldOnly->isChecked())
        args << "FldOnly";
    if (ui->cbDiffOnly->isChecked())
        args << "DiffOnly";
    if (ui->cbIgnoreSetText->isChecked())
        args << "SetDesc=0";
    if (ui->cbCmpDefaults->isChecked())
        args << "CmpDefaults";
    if (ui->cbCmpDomains->isChecked())
        args << "CmpDomains";
    if (ui->cbIgnoreOrder->isChecked())
        args << "IgnoreOrder";
    return args;
}

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbInput1_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose GDX File...",
                                                            mRecentPath,
                                                            ViewHelper::dialogGdxFilter());
    if (!filePath.isEmpty()) {
        mRecentPath = QFileInfo(filePath).path();
        setInput1(filePath);
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbInput2_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose GDX File...",
                                                            mRecentPath,
                                                            ViewHelper::dialogGdxFilter());
    if (!filePath.isEmpty()) {
        mRecentPath = QFileInfo(filePath).path();
        setInput2(filePath);
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbDiff_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Choose GDX File...",
                                                            QDir::cleanPath(mRecentPath +
                                                                QDir::separator() + defaultDiffFile),
                                                                ViewHelper::dialogGdxFilter(),
                                                            nullptr,
                                                            QFileDialog::DontConfirmOverwrite);
    if (!filePath.isEmpty()) {
        mRecentPath = QFileInfo(filePath).path();
        ui->leDiff->setText(QDir::toNativeSeparators(filePath));
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbCancel_clicked()
{
    cancelProcess();
    reject();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbOK_clicked()
{
    mWasCanceled = false;
    mLastInput1 = ui->leInput1->text().trimmed();
    mLastInput2 = ui->leInput2->text().trimmed();
    if (mLastInput1.isEmpty() || mLastInput2.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("GDX Diff");
        msgBox.setText("Please specify two GDX files to be compared.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    if (QFileInfo(mLastInput1).isRelative())
        mLastInput1 = QDir::toNativeSeparators(mWorkingDir + QDir::separator() + mLastInput1);

    if (!QFile(mLastInput1).exists()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("GDX Diff");
        msgBox.setText("Input file (1) does not exist:\n" + mLastInput1);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    if (QFileInfo(mLastInput2).isRelative())
        mLastInput2 = QDir::toNativeSeparators(mWorkingDir + QDir::separator() + mLastInput2);

    if (!QFile(mLastInput2).exists()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("GDX Diff");
        msgBox.setText("Input file (2) does not exist:\n" + mLastInput2);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    mLastDiffFile = ui->leDiff->text().trimmed();
    if (mLastDiffFile.isEmpty())
        mLastDiffFile = QDir::toNativeSeparators(mWorkingDir + QDir::separator() + defaultDiffFile);
    else if (QFileInfo(mLastDiffFile).isRelative())
        mLastDiffFile = QDir::toNativeSeparators(mWorkingDir + QDir::separator() + mLastDiffFile);
    if (QFileInfo(mLastDiffFile).suffix().isEmpty())
        mLastDiffFile = mLastDiffFile + ".gdx";

    if (QFileInfo::exists(mLastDiffFile)) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Overwrite Existing File");
        msgBox.setText(QFileInfo(mLastDiffFile).fileName() + " already exists.\nDo you want to overwrite it?");
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);
        if (msgBox.exec() == QMessageBox::No)
            return;
    }
    setControlsEnabled(false);

    mProc->setWorkingDirectory(mWorkingDir);
    mProc->setParameters(gdxDiffParamters());

    MainWindow* mainWindow = static_cast<MainWindow*>(parent());
    mDiffFm = mainWindow->fileRepo()->fileMeta(QDir::cleanPath(mLastDiffFile));
    if (mDiffFm && !mDiffFm->editors().isEmpty()) {
        mDiffGdxViewer = ViewHelper::toGdxViewer(mDiffFm->editors().first());
        if (mDiffGdxViewer)
            mDiffGdxViewer->releaseFile();
    }
    mProc->execute();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_cbFieldOnly_toggled(bool checked)
{
    if(checked) {
        // uncheck diff only
        ui->cbDiffOnly->setChecked(false);
        // Deselect "All" in the fields-to-compare combo box and select the second entry(L) instead
        if (ui->cbFieldToCompare->currentIndex() == 0)
            ui->cbFieldToCompare->setCurrentIndex(1);
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_cbDiffOnly_toggled(bool checked)
{
    if(checked)
        // uncheck field only
        ui->cbFieldOnly->setChecked(false);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_cbFieldToCompare_currentIndexChanged(int index)
{
    // uncheck the fild only check box if "All" is selected
    if (index==0)
        ui->cbFieldOnly->setChecked(false);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::setRecentPath(const QString &recentPath)
{
    mRecentPath = recentPath;
    if (QFileInfo(ui->leInput1->text()).isFile())
        mWorkingDir = QFileInfo(ui->leInput1->text()).path();
    else
        mWorkingDir = mRecentPath;
    prepopulateDiff();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::setInput1(const QString &filePath)
{
    ui->leInput1->setText(QDir::toNativeSeparators(filePath));
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::setInput2(const QString &filePath)
{
    ui->leInput2->setText(QDir::toNativeSeparators(filePath));
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::reset()
{
    ui->leInput1->clear();
    ui->leInput2->clear();
    ui->lineEdit_4->setText("0.0");
    ui->lineEdit_5->setText("0.0");
    ui->cbDiffOnly->setChecked(false);
    ui->cbFieldOnly->setChecked(false);
    ui->cbIgnoreSetText->setChecked(false);
    ui->cbFieldToCompare->setCurrentIndex(0);
    ui->cbCmpDefaults->setChecked(false);
    ui->cbCmpDomains->setChecked(false);
    ui->cbIgnoreOrder->setChecked(false);
    mPrepopulateDiff = true;
    prepopulateDiff();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbClear_clicked()
{
    reset();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::diffDone()
{
    if (mDiffGdxViewer)
        mDiffGdxViewer->setHasChanged(true);
    if (mDiffFm)
        mDiffFm->reload();
    setControlsEnabled(true);
    if (!mWasCanceled) {
        mWasCanceled = false;
        mLastDiffFile = mProc->diffFile();
        if (mLastDiffFile.isEmpty()) { // give an error pop up that no diff file was created
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("GDX Diff");
            msgBox.setText("Unable to create diff file. gdxdiff return code: " + QString::number(mProc->exitCode()) + ". See the system output for details.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
        accept();
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::setControlsEnabled(bool enabled)
{
    ui->pbOK->setEnabled(enabled);
    ui->pbClear->setEnabled(enabled);
    ui->pbInput1->setEnabled(enabled);
    ui->pbInput2->setEnabled(enabled);
    ui->pbDiff->setEnabled(enabled);
    ui->leDiff->setEnabled(enabled);
    ui->leInput1->setEnabled(enabled);
    ui->leInput2->setEnabled(enabled);
    ui->lineEdit_4->setEnabled(enabled);
    ui->lineEdit_5->setEnabled(enabled);
    ui->cbDiffOnly->setEnabled(enabled);
    ui->cbFieldOnly->setEnabled(enabled);
    ui->cbIgnoreSetText->setEnabled(enabled);
    ui->cbFieldToCompare->setEnabled(enabled);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::cancelProcess(int waitMSec)
{
    if (mProc->state() != QProcess::NotRunning) {
        mWasCanceled = true;
        mProc->stop(waitMSec);
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
    on_pbCancel_clicked();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Hotkey::OpenHelp) {
#ifdef QWEBENGINE
        MainWindow* mainWindow = static_cast<MainWindow*>(parent());
        mainWindow->receiveOpenDoc(help::HelpData::getChapterLocation(help::DocumentType::StudioMain),
                                   help::HelpData::getStudioSectionAnchor(help::HelpData::getStudioSectionName(help::StudioSection::GDXDiff)));

        e->accept();
#endif
        return;
    }
    QDialog::keyPressEvent(e);
}

QString  gams::studio::gdxdiffdialog::GdxDiffDialog::lastInput2() const
{
    return QDir::cleanPath(mLastInput2);
}

QString gams::studio::gdxdiffdialog::GdxDiffDialog::input1() const
{
    return ui->leInput1->text();
}

QString gams::studio::gdxdiffdialog::GdxDiffDialog::input2() const
{
    return ui->leInput2->text();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::prepopulateDiff()
{
    if (mPrepopulateDiff) {
        if (QFileInfo(ui->leInput1->text()).isFile()) {
            QString directory = QFileInfo(ui->leInput1->text()).path();
            mWorkingDir = directory;
            ui->leDiff->setText(QDir::toNativeSeparators(directory + QDir::separator() + defaultDiffFile));
        }
        else
            ui->leDiff->setText(QDir::toNativeSeparators(mWorkingDir + QDir::separator() + defaultDiffFile));
    }
}

QString  gams::studio::gdxdiffdialog::GdxDiffDialog::lastInput1() const
{
    return QDir::cleanPath(mLastInput1);
}

QString  gams::studio::gdxdiffdialog::GdxDiffDialog::lastDiffFile() const
{
    return QDir::cleanPath(mLastDiffFile);
}
