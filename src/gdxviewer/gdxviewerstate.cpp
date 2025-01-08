/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "gdxsymbolviewstate.h"
#include <QVariantMap>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewerState::GdxViewerState()
{

}

GdxViewerState::~GdxViewerState()
{
    for (auto it = mSymbolViewState.begin() ; it != mSymbolViewState.end() ; ) {
        delete it.value();
        it = mSymbolViewState.erase(it);
    }
}

GdxSymbolViewState* GdxViewerState::symbolViewState(const QString& name) const
{
    if (mSymbolViewState.contains(name))
        return mSymbolViewState[name];
    else
        return NULL;
}

GdxSymbolViewState* GdxViewerState::addSymbolViewState(const QString& name)
{
    deleteSymbolViewState(name);
    mSymbolViewState[name] = new GdxSymbolViewState();
    return mSymbolViewState[name];
}

void GdxViewerState::deleteSymbolViewState(const QString& name)
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

QString GdxViewerState::symbolFilter() const
{
    return mSymbolFilter;
}

void GdxViewerState::setSymbolFilter(const QString &newSymbolFilter)
{
    mSymbolFilter = newSymbolFilter;
}


void GdxViewerState::write(QVariantMap &map) const
{
    map.insert("header", mSymbolTableHeaderState.toBase64());
    map.insert("selected", mSelectedSymbol);
    map.insert("isAlias", mSelectedSymbolIsAlias ? 1 : 0);
    map.insert("symbolFilter", mSymbolFilter.toUtf8().toBase64());

    QVariantList symViews;
    QMap<QString, GdxSymbolViewState*>::ConstIterator it;
    for (it = mSymbolViewState.constBegin(); it != mSymbolViewState.constEnd(); ++it) {
        QVariantMap state;
        it.value()->write(state);
        state.insert("name", it.key());
        symViews.append(state);
    }
    map.insert("symbolViewStates", symViews);
}

void GdxViewerState::read(const QVariantMap &map)
{
    mSymbolTableHeaderState = QByteArray::fromBase64(map.value("header").toByteArray());
    mSelectedSymbol = map.value("selected").toString();
    mSelectedSymbolIsAlias = map.value("isAlias").toInt();
    mSymbolFilter = QByteArray::fromBase64(map.value("symbolFilter").toByteArray());

    if (map.contains("symbolViewStates")) {
        for (auto it = mSymbolViewState.begin(); it != mSymbolViewState.end();)
            it = mSymbolViewState.erase(it);

        const QVariantList symViews = map.value("symbolViewStates").toList();
        for (const QVariant &value : symViews) {
            QVariantMap map = value.toMap();
            GdxSymbolViewState *symView = new GdxSymbolViewState();
            symView->read(map);
            mSymbolViewState.insert(map.value("name").toString(), symView);
        }

    }
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
