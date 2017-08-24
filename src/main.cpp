#include "gamside.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GAMSIDE w;
    w.show();

    return a.exec();
}
