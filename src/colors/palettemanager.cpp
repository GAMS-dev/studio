#include <QApplication>
#include <QStyleFactory>

#include "palettemanager.h"

namespace gams {
namespace studio {

PaletteManager* PaletteManager::mInstance = nullptr;

PaletteManager::PaletteManager()
{
    // save original style for light theme (windows)
    mDefaultSyle = QApplication::style()->objectName();

    // Nr1: default style
    mStyles.append(QApplication::palette());

    // Nr2: dark theme
    QPalette darkPalette(QApplication::palette());
    QColor darkColor = QColor(42,42,42);
    QColor disabledColor = QColor(127,127,127);
    darkPalette.setColor(QPalette::Window, darkColor);
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(30,30,30));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66,66,66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
    darkPalette.setColor(QPalette::Button, darkColor);
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42,130,218));
    darkPalette.setColor(QPalette::Highlight, QColor(243,150,25));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
    mStyles.append(darkPalette);

    // Nr3: insert here
}

PaletteManager::~PaletteManager()
{
    delete mInstance;
}

PaletteManager *PaletteManager::instance()
{
    if (!mInstance) mInstance = new PaletteManager();

    return mInstance;
}

void PaletteManager::setPalette(int i)
{
    if (i >= 0 && i < nrPalettes()) {
        mActivePalette = i;
        applyPalette(i);
    }
}

int PaletteManager::nrPalettes()
{
    return mStyles.size();
}

int PaletteManager::activePalette()
{
    return mActivePalette;
}

void PaletteManager::applyPalette(int i)
{
    QPalette p = mStyles.at(i);
    if (i == 1)
        QApplication::setStyle(QStyleFactory::create("Fusion")); // this needs to be set so everything turns dark
    else
        QApplication::setStyle(QStyleFactory::create(mDefaultSyle));
    QApplication::setPalette(p);
}

}
}

