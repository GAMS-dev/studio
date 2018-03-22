#ifndef GAMS_STUDIO_SELECTENCODINGS_H
#define GAMS_STUDIO_SELECTENCODINGS_H

#include <QDialog>

namespace Ui {
class SelectEncodings;
}

namespace gams {
namespace studio {

class SelectEncodings : public QDialog
{
    Q_OBJECT

public:
    explicit SelectEncodings(QList<int> selectedMibs, QWidget *parent = 0);
    ~SelectEncodings();
    QList<int> selectedMibs();

private slots:

    void on_pbCancel_clicked();

    void on_pbSave_clicked();

    void on_pbReset_clicked();

private:
    Ui::SelectEncodings *ui;
};

}
}

#endif // GAMS_STUDIO_SELECTENCODINGS_H
