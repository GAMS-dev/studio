#ifndef REFERENCEWIDGET_H
#define REFERENCEWIDGET_H

#include <QWidget>
#include <QMap>

#include "symbolreferenceitem.h"

namespace Ui {
class ReferenceWidget;
}

namespace gams {
namespace studio {


class ReferenceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReferenceWidget(QString referenceFile, QWidget *parent = nullptr);
    ~ReferenceWidget();

private:
    Ui::ReferenceWidget *ui;

    bool parseFile(QString referenceFile);
    void addReferenceInfo(SymbolReferenceItem* ref, const QString &referenceType, int lineNumber, int columnNumber, const QString &location);

    bool mValid;
    QString mReferenceFile;
    QMap<QString, SymbolId> mSymbolNameMap;
    QMap<SymbolId, SymbolReferenceItem*> mReference;

};

} // namespace studio
} // namespace gamsq

#endif // REFERENCEWIDGET_H
