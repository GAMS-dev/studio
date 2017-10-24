#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H

#include <QAbstractItemModel>
#include "gdxsymbol.h"
#include <memory>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolTable : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GdxSymbolTable(gdxHandle_t gdx, QObject *parent = 0);
    ~GdxSymbolTable();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QList<GdxSymbol *> gdxSymbols() const;
    QString uel2Label(int uel);

private:
    QStringList mHeaderText;

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


};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H
