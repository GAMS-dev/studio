#include "gdxviewer.h"
#include "gdxsymboltable.h"
#include "gdxsymbol.h"
#include "gdxsymbolview.h"
#include "exception.h"
#include <memory>
#include <QtConcurrent>
#include <QFutureWatcher>

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
        //TODO(CW): raise exception wit proper message
        EXCEPT() << "Could not load GDX library: " << msg;
    }

    gdxOpenRead(mGdx, gdxFile.toLatin1(), &errNr);
    if (errNr) reportIoError(errNr,"gdxOpenRead");

    mGdxSymbolTable = new GdxSymbolTable(mGdx, mGdxMutex);
    mSymbolViews.resize(mGdxSymbolTable->symbolCount());

    ui.tvSymbols->setModel(mGdxSymbolTable);
    ui.tvSymbols->resizeColumnsToContents();

    connect(ui.tvSymbols->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GdxViewer::updateSelectedSymbol);
}

GdxViewer::~GdxViewer()
{
    GdxSymbol* selected = selectedSymbol();
    if(selected)
        selected->stopLoadingData();

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
        if(!selectedSymbol->isLoaded())
            QtConcurrent::run(this, &GdxViewer::loadSymbol, selectedSymbol);

        // create new GdxSymbolView if the symbol is selected for the first time
        if(!mSymbolViews.at(selected.indexes().at(0).row()))
        {
            GdxSymbolView* symView = new GdxSymbolView();
            mSymbolViews.replace(selected.indexes().at(0).row(), symView);
            symView->setSym(selectedSymbol);
        }
        ui.splitter->replaceWidget(1, mSymbolViews.at(selected.indexes().at(0).row()));
    }
    else
    {
        ui.splitter->replaceWidget(1, ui.gdxSymbolView);
    }
}

GdxSymbol *GdxViewer::selectedSymbol()
{
    GdxSymbol* selected = nullptr;
    QModelIndexList selectedIdx = ui.tvSymbols->selectionModel()->selectedRows();
    if(!selectedIdx.isEmpty())
        selected = mGdxSymbolTable->gdxSymbols().at(selectedIdx.at(0).row());
    return selected;
}

void GdxViewer::loadSymbol(GdxSymbol* selectedSymbol)
{
    selectedSymbol->loadData();
    emit loadFinished();
}

void GdxViewer::reportIoError(int errNr, QString message)
{
    // TODO(JM) An exception contains information about it's source line -> it should be thrown where it occurs
    EXCEPT() << "Fatal I/O Error = " << errNr << " when calling " << message;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
