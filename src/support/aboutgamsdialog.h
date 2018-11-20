#ifndef ABOUTGAMSDIALOG_H
#define ABOUTGAMSDIALOG_H

#include <QDialog>


namespace gams {
namespace studio {
namespace support {

namespace Ui {
class AboutGAMSDialog;
}

class AboutGAMSDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutGAMSDialog(QWidget *parent = nullptr);
    ~AboutGAMSDialog();
    QString studioInfo();
    QString aboutStudio();
    QString licenseInformation();

private slots:
    void on_copylicense_clicked();
    void on_close_clicked();

private:
    Ui::AboutGAMSDialog *ui;
};

}
}
}
#endif // ABOUTGAMSDIALOG_H
