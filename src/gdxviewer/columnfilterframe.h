#ifndef GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H
#define GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H

#include "ui_columnfilterframe.h"
#include "gdxsymbol.h"
#include "filteruelmodel.h"
#include <QVector>

namespace gams {
namespace studio {
namespace gdxviewer {

class ColumnFilterFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ColumnFilterFrame(GdxSymbol* symbol, int column, QWidget *parent = 0);
    ~ColumnFilterFrame();

protected:
    //mouse events overwritten to prevent closing of the filter menu if user click on empty spaces regions within the frame
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;

private:
    Ui::ColumnFilterFrame ui;
    GdxSymbol* mSymbol;
    int mColumn;
    FilterUelModel* mModel;

private slots:
    void apply();
    void selectAll();
    void deselectAll();
    void filterLabels();
    void toggleHideUnselected(bool checked);
    void listDataHasChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H
