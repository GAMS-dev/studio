#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <QWidget>

namespace Ui {
class WelcomePage;
}

namespace gams {
namespace ide {

class WelcomePage : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = 0);
    ~WelcomePage();

private slots:
    void labelLinkActivated(const QString &link);

private:
    Ui::WelcomePage *ui;
};

}
}

#endif // WELCOMEPAGE_H
