#include "gdxsymbolview.h"
#include "ui_gdxsymbolview.h"
#include "gdxsymbolheaderview.h"
#include "columnfilter.h"
#include <QMenu>

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolView::GdxSymbolView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GdxSymbolView)
{
    ui->setupUi(this);
    GdxSymbolHeaderView* headerView = new GdxSymbolHeaderView(Qt::Horizontal);
    headerView->setEnabled(false);

    ui->tableView->setHorizontalHeader(headerView);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->horizontalHeader()->setSortIndicatorShown(true);
    ui->tableView->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    ui->tableView->horizontalHeader()->setSectionsClickable(true);
    ui->tableView->horizontalHeader()->setSectionsMovable(true);
    ui->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->tableView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &GdxSymbolView::showColumnFilter);
    connect(ui->cbSqueezeDefaults, &QCheckBox::toggled, this, &GdxSymbolView::toggleSqueezeDefaults);
    connect(ui->pbResetSortFilter, &QPushButton::clicked, this, &GdxSymbolView::resetSortFilter);
}

GdxSymbolView::~GdxSymbolView()
{
    delete ui;
}

void GdxSymbolView::showColumnFilter(QPoint p)
{
    int column = ui->tableView->horizontalHeader()->logicalIndexAt(p);
    if(mSym->isLoaded() && column>=0 && column<mSym->dim())
    {
        QMenu m(this);
        m.addAction(new ColumnFilter(mSym, column, this));
        m.exec(ui->tableView->mapToGlobal(p));
    }
}


void GdxSymbolView::toggleSqueezeDefaults(bool checked)
{
    if(mSym)
    {
        ui->tableView->setUpdatesEnabled(false);
        if(checked)
        {
            for(int i=0; i<GMS_VAL_MAX; i++)
            {
                if (mSym->isAllDefault(i))
                    ui->tableView->setColumnHidden(mSym->dim()+i, true);
                else
                    ui->tableView->setColumnHidden(mSym->dim()+i, false);
            }
        }
        else
        {
            for(int i=0; i<GMS_VAL_MAX; i++)
                ui->tableView->setColumnHidden(mSym->dim()+i, false);
        }
        ui->tableView->setUpdatesEnabled(true);
    }
}

void GdxSymbolView::resetSortFilter()
{
    if(mSym)
    {
        mSym->resetSortFilter();
        ui->tableView->horizontalHeader()->restoreState(mInitialHeaderState);
    }
}

void GdxSymbolView::refreshView()
{
    if(!mSym)
        return;
    if(mSym->isLoaded())
        mSym->filterRows();
}


GdxSymbol *GdxSymbolView::sym() const
{
    return mSym;
}

void GdxSymbolView::setSym(GdxSymbol *sym)
{
    mSym = sym;
    if(mSym->recordCount()>0) //enable controls only for symbols that have records, otherwise it does not make sense to filter, sort, etc
        connect(mSym, &GdxSymbol::loadFinished, this, &GdxSymbolView::enableControls);
    ui->tableView->setModel(mSym);
    refreshView();
}

void GdxSymbolView::enableControls()
{
    ui->tableView->horizontalHeader()->setEnabled(true);
    mInitialHeaderState = ui->tableView->horizontalHeader()->saveState();
    if(mSym->type() == GMS_DT_VAR || mSym->type() == GMS_DT_EQU)
        ui->cbSqueezeDefaults->setEnabled(true);
    else
        ui->cbSqueezeDefaults->setEnabled(false);
    ui->pbResetSortFilter->setEnabled(true);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
