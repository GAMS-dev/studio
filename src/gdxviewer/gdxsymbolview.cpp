#include "gdxsymbolview.h"
#include "ui_gdxsymbolview.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxSymbolView::GdxSymbolView(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::GdxSymbolView)
{
    ui->setupUi(this);
}

GdxSymbolView::GdxSymbolView(GdxSymbol *sym, QWidget *parent)
    : GdxSymbolView(parent), mSym(sym)
{

}

GdxSymbolView::~GdxSymbolView()
{
    delete ui;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
