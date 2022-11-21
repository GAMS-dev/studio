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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWERSTATE_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWERSTATE_H

#include "gdxsymbolviewstate.h"

#include <QMap>
#include <QObject>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewerState
{
public:
    GdxViewerState();
    ~GdxViewerState();

    GdxSymbolViewState* symbolViewState(QString name) const;
    GdxSymbolViewState* addSymbolViewState(QString name);
    void deleteSymbolViewState(QString name);

    QByteArray symbolTableHeaderState() const;
    void setSymbolTableHeaderState(const QByteArray &symbolTableHeaderState);

    QMap<QString, GdxSymbolViewState *> symbolViewStates() const;

    QString selectedSymbol() const;
    void setSelectedSymbol(const QString &selectedSymbol);

    bool selectedSymbolIsAlias() const;
    void setSelectedSymbolIsAlias(bool selectedSymbolIsAlias);

    void write(QVariantMap &map);
    void read(const QVariantMap &map);

private:
    QByteArray mSymbolTableHeaderState;
    QMap<QString, GdxSymbolViewState*> mSymbolViewState;
    QString mSelectedSymbol = "";
    bool mSelectedSymbolIsAlias = false;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWERSTATE_H
