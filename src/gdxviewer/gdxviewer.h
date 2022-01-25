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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H

#include <QWidget>
#include <QVector>
#include <QItemSelection>
#include <QTextCodec>

#include "gdxcc.h"
#include "common.h"

class QMutex;
class QSortFilterProxyModel;

namespace gams {
namespace studio {
namespace gdxviewer {

namespace Ui {
class GdxViewer;
}

class GdxSymbol;
class GdxSymbolTable;
class GdxSymbolView;

class GdxViewer : public QWidget
{
    Q_OBJECT

public:
    GdxViewer(QString gdxFile, QString systemDirectory, QTextCodec* codec, QWidget *parent = nullptr);
    ~GdxViewer();
    void updateSelectedSymbol(QItemSelection selected, QItemSelection deselected);
    GdxSymbol* selectedSymbol();
    int reload(QTextCodec* codec, bool quiet = false);
    void setHasChanged(bool value);
    void copyAction();
    void selectAllAction();
    void selectSearchField();
    void releaseFile();
    void invalidate();

private slots:
    void hideUniverseSymbol();
    void toggleSearchColumns(bool checked);

private:
    void loadSymbol(GdxSymbol* selectedSymbol);
    void copySelectionToClipboard();
    int init(bool quiet = false);
    void freeSymbols();
    bool mIsInitialized = false;

    static int errorCallback(int count, const char *message);

private:
    Ui::GdxViewer *ui;

    QString mGdxFile;
    QString mSystemDirectory;

    bool mHasChanged = false;

    GdxSymbolTable* mGdxSymbolTable = nullptr;
    QSortFilterProxyModel* mSymbolTableProxyModel = nullptr;

    gdxHandle_t mGdx;
    QMutex* mGdxMutex = nullptr;

    QVector<GdxSymbolView*> mSymbolViews;

    QTextCodec *mCodec;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
