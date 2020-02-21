#ifndef PALETTEMANAGER_H
#define PALETTEMANAGER_H

#include <QObject>

#include "mainwindow.h"

namespace gams {
namespace studio {

class PaletteManager : QObject
{
    Q_OBJECT
public:
    PaletteManager();
    ~PaletteManager();

    static PaletteManager* instance();

    void setPalette(int i);
    int getNrPalettes();
    int getActivePalette();

private:
    int activePalette;
    QList<QPalette> mStyles;
    static PaletteManager* mInstance;
    int mActivePalette = -1;

    void applyPalette(int i);
};


}
}
#endif // PALETTEMANAGER_H
