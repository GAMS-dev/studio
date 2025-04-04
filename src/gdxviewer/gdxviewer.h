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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H

#include <QVector>
#include <QItemSelection>

#include "abstractview.h"
#include "gdxcc.h"
#include "common.h"
#include "gdxviewerstate.h"
#include "exportdialog.h"

class QMutex;
class QSortFilterProxyModel;

namespace gams {
namespace studio {
namespace gdxviewer {

namespace Ui {
class GdxViewer;
}

class GdxSymbol;
class GdxSymbolTableModel;
class GdxSymbolView;

class GdxViewer : public AbstractView
{
    Q_OBJECT

public:
    GdxViewer(const QString &gdxFile, const QString &systemDirectory, const QString &encoding, QWidget *parent = nullptr);
    ~GdxViewer() override;
    void updateSelectedSymbol(const QItemSelection &selected, const QItemSelection& deselected);
    GdxSymbol* selectedSymbol();
    void createSymbolView(GdxSymbol* sym, int symbolIndex);
    void setHasChanged(bool value);
    void copyAction();
    void selectAllAction();
    void selectSearchField();
    void releaseFile();
    bool dragInProgress();
    GdxSymbolView *symbolViewByName(QString name);
    QString gdxFile() const;
    GdxViewerState *state() const;
    void readState(const QVariantMap& map);
    void writeState(const QString &location);
    QStringList getEnabledContextActions();
    GdxSymbolTableModel *gdxSymbolTable() const;
    void saveDelete();

public slots:
    void invalidate();
    int reload(const QString &encoding, bool quiet=false, bool triggerReload=true);

private slots:
    void hideUniverseSymbol();
    void columnScopeChanged();
    void applySelectedSymbolOnFocus(QWidget *old, QWidget *now);
    void on_pbExport_clicked();

private:
    void loadSymbol(GdxSymbol* selectedSymbol);
    void copySelectionToClipboard();
    int init(bool quiet = false);
    void freeSymbols();
    bool isFocusedWidget(QWidget *wid);
    void zoomInF(qreal range);
    void setFilter(const QString &text);

    static int errorCallback(int count, const char *message);

private:
    void saveState();
    void applyState();
    void applySymbolState(GdxSymbol* symbol);
    void applySelectedSymbol();
    void showExportDialog();

    Ui::GdxViewer *ui;
    QString mGdxFile;
    QString mSystemDirectory;
    bool mIsInitialized = false;
    bool mHasChanged = false;

    GdxSymbolTableModel* mGdxSymbolTable = nullptr;
    QSortFilterProxyModel* mSymbolTableProxyModel = nullptr;

    gdxHandle_t mGdx;
    QMutex* mGdxMutex = nullptr;

    QVector<GdxSymbolView*> mSymbolViews;

    QStringDecoder mDecoder;

    GdxViewerState* mState = nullptr;
    bool mPendingInvalidate = false;
    ExportDialog *mExportDialog = nullptr;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
