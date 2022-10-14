/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include "gdxviewerstate.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewerState::GdxViewerState()
{

}

GdxViewerState::~GdxViewerState()
{
    for (QString name : mSymbolViewState.keys()) {
        deleteSymbolViewState(name);
    }
}

GdxSymbolViewState* GdxViewerState::symbolViewState(QString name) const
{
    if (mSymbolViewState.contains(name))
        return mSymbolViewState[name];
    else
        return NULL;
}

GdxSymbolViewState* GdxViewerState::addSymbolViewState(QString name)
{
    deleteSymbolViewState(name);
    mSymbolViewState[name] = new GdxSymbolViewState();
    return mSymbolViewState[name];
}

void GdxViewerState::deleteSymbolViewState(QString name)
{
    if (mSymbolViewState.contains(name)) {
        delete mSymbolViewState[name];
        mSymbolViewState.remove(name);
    }
}

QByteArray GdxViewerState::symbolTableHeaderState() const
{
    return mSymbolTableHeaderState;
}

void GdxViewerState::setSymbolTableHeaderState(const QByteArray &symbolTableHeaderState)
{
    mSymbolTableHeaderState = symbolTableHeaderState;
}

QMap<QString, GdxSymbolViewState *> GdxViewerState::symbolViewStates() const
{
    return mSymbolViewState;
}

QString GdxViewerState::selectedSymbol() const
{
    return mSelectedSymbol;
}

void GdxViewerState::setSelectedSymbol(const QString &selectedSymbol)
{
    mSelectedSymbol = selectedSymbol;
}

bool GdxViewerState::selectedSymbolIsAlias() const
{
    return mSelectedSymbolIsAlias;
}

void GdxViewerState::setSelectedSymbolIsAlias(bool selectedSymbolIsAlias)
{
    mSelectedSymbolIsAlias = selectedSymbolIsAlias;
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
