#include "gdxviewer.h"
#include "gdxsymboltable.h"
#include "gdxsymbol.h"
#include "exception.h"
#include <memory>
#include <QtConcurrent>
#include <QTime>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewer::GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent) :
    QFrame(parent)
{
    ui.setupUi(this);
    ui.splitter->setStretchFactor(0,1);
    ui.splitter->setStretchFactor(1,2);

    mGdxMutex = new QMutex();

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

    mGdxSymbolTable = new GdxSymbolTable(mGdx, mGdxMutex);

    ui.tvSymbols->setModel(mGdxSymbolTable);
    ui.tvSymbols->resizeColumnsToContents();

    connect(ui.tvSymbols->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GdxViewer::updateSelectedSymbol);
}

GdxViewer::~GdxViewer()
{
    QModelIndexList selectedIdx = ui.tvSymbols->selectionModel()->selectedRows();
    if(selectedIdx.size()>0)
        mGdxSymbolTable->gdxSymbols().at(selectedIdx.at(0).row())->stopLoadingData();

    delete mGdxSymbolTable;

    QMutexLocker locker(mGdxMutex);
    gdxClose(mGdx);
    gdxFree(&mGdx);
    locker.unlock();
    delete mGdxMutex;
}

void GdxViewer::updateSelectedSymbol(QItemSelection selected, QItemSelection deselected)
{
    if (selected.indexes().size()>0)
    {
        if (deselected.indexes().size()>0)
        {
            GdxSymbol* deselectedSymbol = mGdxSymbolTable->gdxSymbols().at(deselected.indexes().at(0).row());
            QtConcurrent::run(deselectedSymbol, &GdxSymbol::stopLoadingData);
        }
        GdxSymbol* selectedSymbol = mGdxSymbolTable->gdxSymbols().at(selected.indexes().at(0).row());
        QtConcurrent::run(selectedSymbol, &GdxSymbol::loadData);
        ui.tableView->setModel(selectedSymbol);
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
