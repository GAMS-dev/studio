#ifndef PALETTEMANAGER_H
#define PALETTEMANAGER_H

#include <QObject>

#include "mainwindow.h"

namespace gams {
namespace studio {

class PaletteManager : public QObject
{
    Q_OBJECT
public:
    PaletteManager();
    ~PaletteManager();

    static PaletteManager* instance();

    void setPalette(int i);
    int nrPalettes();
    int activePalette();

private:
    QList<QPalette> mStyles;
    static PaletteManager* mInstance;
    int mActivePalette = -1;
    QString mDefaultStyle;

    void applyPalette(int i);
};


}
}
#endif // PALETTEMANAGER_H
