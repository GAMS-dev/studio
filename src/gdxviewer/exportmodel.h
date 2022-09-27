#ifndef GAMS_STUDIO_GDXVIEWER_EXPORTMODEL_H
#define GAMS_STUDIO_GDXVIEWER_EXPORTMODEL_H

#include <QAbstractTableModel>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewer;
class GdxSymbolTableModel;
class GdxSymbol;

class ExportModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ExportModel(GdxViewer* gdxViewer, GdxSymbolTableModel *symbolTableModel, QObject *parent = nullptr);

    ~ExportModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QList<GdxSymbol*> selectedSymbols();

    QStringList range() const;

private:
    GdxSymbolTableModel *mSymbolTableModel = nullptr;
    QVector<bool> mChecked;
    QStringList mRange;
    GdxViewer *mGdxViewer = nullptr;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_EXPORTMODEL_H
