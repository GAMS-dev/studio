/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <QApplication>
#include <QStyleFactory>

#include "palettemanager.h"

namespace gams {
namespace studio {

PaletteManager* PaletteManager::mInstance = nullptr;

PaletteManager::PaletteManager()
{
    // save original style for light theme (windows)
    mDefaultStyle = QApplication::style()->objectName();

    // Nr1: default style
    auto p = QApplication::palette();
    QColor back = p.color(QPalette::Highlight);
    p.setColor(QPalette::Active, QPalette::Highlight, p.color(QPalette::Highlight));
    p.setColor(QPalette::Active, QPalette::HighlightedText, p.color(QPalette::HighlightedText));
    QColor gray = QColor(180,180,180);
    back = QColor((2*back.red()+gray.red())/3, (2*back.green()+gray.green())/3, (2*back.blue()+gray.blue())/3);
    p.setColor(QPalette::Inactive, QPalette::Highlight, back);
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, p.color(QPalette::HighlightedText));

    mStyles.append(p);

    // Nr2: dark theme
    QPalette darkPalette(QApplication::palette());
    QColor disabledColor = QColor(127,127,127);
    darkPalette.setColor(QPalette::Window, QColor(30,30,30));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
    darkPalette.setColor(QPalette::PlaceholderText, disabledColor);
    darkPalette.setColor(QPalette::Base, QColor(45,45,45));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66,66,66));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(42,42,42));
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Light, QColor(0, 0, 0, 0)); // removes white text shadow
    darkPalette.setColor(QPalette::Button, QColor(50,50,50));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42,130,218));
    QColor gamsOrange = QColor(243,150,25);
    darkPalette.setColor(QPalette::Highlight, gamsOrange);
    gamsOrange.setAlpha(220);
    darkPalette.setColor(QPalette::Inactive, QPalette::Highlight, gamsOrange);
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
    mStyles.append(darkPalette);

    // Nr3: insert here
}

PaletteManager::~PaletteManager()
{
}

PaletteManager *PaletteManager::instance()
{
    if (!mInstance) mInstance = new PaletteManager();

    return mInstance;
}

void PaletteManager::deleteInstance()
{
    if (mInstance) delete mInstance;
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
    qApp->setPalette(p);
    emit paletteChanged();
}

}
}

