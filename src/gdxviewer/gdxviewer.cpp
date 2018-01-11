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

    //TODO: delete GdxSymbolViews
}

void GdxViewer::updateSelectedSymbol(QItemSelection selected, QItemSelection deselected)
{
    if (selected.indexes().size()>0)
    {
        int selectedIdx = selected.indexes().at(0).row();
        if (deselected.indexes().size()>0)
        {
            GdxSymbol* deselectedSymbol = mGdxSymbolTable->gdxSymbols().at(deselected.indexes().at(0).row());
            QtConcurrent::run(deselectedSymbol, &GdxSymbol::stopLoadingData);
        }
        GdxSymbol* selectedSymbol = mGdxSymbolTable->gdxSymbols().at(selectedIdx);

        //aliases are also aliases in the sense of the view
        if(selectedSymbol->type() == GMS_DT_ALIAS)
        {
            selectedIdx = selectedSymbol->subType() - 1;
            selectedSymbol = mGdxSymbolTable->gdxSymbols().at(selectedIdx);
        }

        if(!selectedSymbol->isLoaded())
            QtConcurrent::run(this, &GdxViewer::loadSymbol, selectedSymbol);

        // create new GdxSymbolView if the symbol is selected for the first time
        if(!mSymbolViews.at(selectedIdx))
        {
            GdxSymbolView* symView = new GdxSymbolView();
            mSymbolViews.replace(selectedIdx, symView);
            symView->setSym(selectedSymbol);
        }
        ui.splitter->replaceWidget(1, mSymbolViews.at(selectedIdx));
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
