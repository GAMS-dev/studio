#ifndef APPLICATION_H
#define APPLICATION_H

#include <QtWidgets>

namespace gams {
namespace studio {

class Application : public QApplication
{
public:
    Application(int &argc, char **argv);
    bool notify(QObject *object, QEvent *event) override;
    static void showBox(QString title, QString message);
private:
};

} // namespace studio
} // namespace gams

#endif // APPLICATION_H
