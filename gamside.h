#ifndef GAMSIDE_H
#define GAMSIDE_H

#include <QMainWindow>

namespace Ui {
class GAMSIDE;
}

class GAMSIDE : public QMainWindow
{
    Q_OBJECT

public:
    explicit GAMSIDE(QWidget *parent = 0);
    ~GAMSIDE();

private slots:
    void on_actionExit_Application_triggered();

    void on_actionOnline_Help_triggered();

private:
    Ui::GAMSIDE *ui;
};

#endif // GAMSIDE_H
