#include "gdxviewer.h"
#include "gdxsymboltable.h"
#include "gdxsymbol.h"
#include "exception.h"
#include <memory>
#include <QtConcurrent>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewer::GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent) :
    QFrame(parent)
{
    ui.setupUi(this);
    ui.splitter->setStretchFactor(0,1);
    ui.splitter->setStretchFactor(1,2);

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

    mGdxSymbolTable = new GdxSymbolTable(mGdx);

    ui.tvSymbols->setModel(mGdxSymbolTable);
    ui.tvSymbols->resizeColumnsToContents();

    connect(ui.tvSymbols->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GdxViewer::updateSelectedSymbol);
}

GdxViewer::~GdxViewer()
{
    delete mGdxSymbolTable;
    gdxClose(mGdx);
    gdxFree(&mGdx);
}

void GdxViewer::updateSelectedSymbol()
{
    QModelIndexList modelIndexList = ui.tvSymbols->selectionModel()->selectedIndexes();
    if(modelIndexList.size()>0)
    {
        GdxSymbol* sym = mGdxSymbolTable->gdxSymbols().at(modelIndexList.at(0).row());
        QtConcurrent::run(sym, GdxSymbol::loadData);
        ui.tableView->setModel(sym);
    }
    else
        ui.tableView->setModel(nullptr);
}

void GdxViewer::reportIoError(int errNr, QString message)
{
    //TODO(CW): proper Exception message and remove qDebug
    qDebug() << "**** Fatal I/O Error = " << errNr << " when calling " << message;
    throw Exception();
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
