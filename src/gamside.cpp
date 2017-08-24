#include "gamside.h"
#include "ui_gamside.h"
#include <QDesktopServices>
#include <QUrl>

GAMSIDE::GAMSIDE(QWidget *parent) : QMainWindow(parent), ui(new Ui::GAMSIDE)
{
    ui->setupUi(this);
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
