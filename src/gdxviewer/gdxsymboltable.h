#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H

#include <QAbstractItemModel>
#include <QMutex>
#include "gdxsymbol.h"
#include <memory>
#include "gdxcc.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbol;

class GdxSymbolTable : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GdxSymbolTable(gdxHandle_t gdx, QMutex* gdxMutex, QObject *parent = 0);
    ~GdxSymbolTable();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QList<GdxSymbol *> gdxSymbols() const;

    QStringList uel2Label() const;

    QStringList strPool() const;

    int *labelCompIdx();

    int symbolCount() const;

private:
    QStringList mHeaderText;

    QString typeAsString(int type) const;
    void createSortIndex();

    gdxHandle_t mGdx = nullptr;
    int mUelCount;
    int mSymbolCount;
    void loadUel2Label();
    void loadStringPool();
    void loadGDXSymbols();
    void reportIoError(int errNr, QString message);

    QList<GdxSymbol*> mGdxSymbols;
    QStringList mUel2Label;
    QStringList mStrPool;

    int* mLabelCompIdx = nullptr;

    QMutex* mGdxMutex;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H
