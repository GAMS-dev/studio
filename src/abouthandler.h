#ifndef ABOUTHANDLER_H
#define ABOUTHANDLER_H

#include <QDialog>

namespace Ui {
class AboutHandler;
}

namespace gams {
namespace studio {

// TODO(AF): rename to AboutDialog or AboutGAMSDialog
class AboutHandler : public QDialog
{
    Q_OBJECT

public:
    explicit AboutHandler(QWidget *parent = nullptr);
    ~AboutHandler();
    QString studioInfo();
    QString aboutStudio();
    QString licenseInformation();

private slots:
    void on_copylicense_clicked();
    void on_close_clicked();

private:
    Ui::AboutHandler *ui;
};

}
}
#endif // ABOUTHANDLER_H
