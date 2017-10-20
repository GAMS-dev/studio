#include "gdxviewer.h"
#include "gdxsymboltablemodel.h"
#include "gdxsymboldatatablemodel.h"
#include "exception.h"
#include <memory>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewer::GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent) :
    QFrame(parent)
{
    ui.setupUi(this);
    char msg[GMS_SSSIZE];
    int errNr = 0;
    if (!gdxCreateD(&mGdx, systemDirectory.toLatin1(), msg, sizeof(msg)))
    {
        //TODO(CW): raise exception wit proper message and remove the cout
        qDebug() << "**** Could not load GDX library";
        qDebug() << "**** " << msg;
        throw Exception();
    }

    gdxOpenRead(mGdx, gdxFile.toLatin1(), &errNr);
    if (errNr) reportIoError(errNr,"gdxOpenRead");

    mGdxSymbols = loadGDXSymbol();
    mUel2Label = loadUel2Label();

    gdxClose(mGdx);
    gdxFree(&mGdx);

    ui.tvSymbols->setModel(new GdxSymbolTableModel(mGdxSymbols));
    connect(ui.tvSymbols->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GdxViewer::updateSelectedSymbol);
}

GdxViewer::~GdxViewer()
{
    qDebug() << "destructor of GdxViewer";
}

void GdxViewer::updateSelectedSymbol()
{
    QModelIndexList modelIndexList = ui.tvSymbols->selectionModel()->selectedIndexes();
    if(modelIndexList.size()>0)
    {
        ui.tableView->setModel(new GDXSymbolDataTableModel(mGdxSymbols.at(modelIndexList.at(0).row()), &mUel2Label));
    }
    else
    {

    }
}

void GdxViewer::reportIoError(int errNr, QString message)
{
    //TODO(CW): proper Exception message and remove qDebug
    qDebug() << "**** Fatal I/O Error = " << errNr << " when calling " << message;
    throw Exception();
}

QList<std::shared_ptr<GDXSymbol> > GdxViewer::loadGDXSymbol()
{
    int symbolCount = 0;
    gdxSystemInfo(mGdx, &symbolCount, &mUelCount);
    QList<std::shared_ptr<GDXSymbol>> symbols;
    for(int i=1; i<symbolCount+1; i++)
    {
        char symName[GMS_SSSIZE];
        char explText[GMS_SSSIZE];
        int dimension = 0;
        int type = 0;
        gdxSymbolInfo(mGdx, i, symName, &dimension, &type);
        int recordCount = 0;
        int userInfo = 0;
        gdxSymbolInfoX (mGdx, i, &recordCount, &userInfo, explText);
        std::shared_ptr<GDXSymbol> sym = std::make_shared<GDXSymbol>(i, QString(symName), dimension, type, userInfo, recordCount, QString(explText), mGdx);
        sym->loadData();
        symbols.append(sym);
    }
    return symbols;
}

QStringList GdxViewer::loadUel2Label()
{
    QStringList uel2Label;
    char label[GMS_SSSIZE];
    int map;
    for(int i=0; i<=mUelCount; i++)
    {
        gdxUMUelGet(mGdx, i, label, &map);
        uel2Label.append(QString(label));
    }
    return uel2Label;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
