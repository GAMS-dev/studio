#include "gamside.h"
#include "ui_gamside.h"
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>

GAMSIDE::GAMSIDE(QWidget *parent) : QMainWindow(parent), ui(new Ui::GAMSIDE)
{
    ui->setupUi(this);
    ui->dockBottom->hide();
}

GAMSIDE::~GAMSIDE()
{
    delete ui;
}

void GAMSIDE::on_actionExit_Application_triggered()
{
    QCoreApplication::quit();
}

void GAMSIDE::on_actionOnline_Help_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.gams.com/latest/docs/welcome.html", QUrl::TolerantMode));
}

void GAMSIDE::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About GAMSIDE", "Gams Studio v0.0.1 alpha");
}

void GAMSIDE::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void GAMSIDE::on_actionProject_Explorer_triggered(bool checked)
{
    if(checked)
        ui->dockProjectExplorer->show();
    else
        ui->dockProjectExplorer->hide();
}

void GAMSIDE::on_actionLog_Output_triggered(bool checked)
{
    if(checked)
        ui->dockOutput->show();
    else
        ui->dockOutput->hide();
}

void GAMSIDE::on_actionBottom_Panel_triggered(bool checked)
{
    if(checked)
        ui->dockBottom->show();
    else
        ui->dockBottom->hide();
}
