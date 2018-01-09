#ifndef RESULTSVIEW_H
#define RESULTSVIEW_H

#include <QWidget>

namespace Ui {
class ResultsView;
}

namespace gams {
namespace studio {

class Result;
class ResultsView : public QWidget
{
    Q_OBJECT

public:
    explicit ResultsView(QWidget *parent = 0);
    ~ResultsView();
    void addItem(Result r);
    void resizeColumnsToContent();

private:
    Ui::ResultsView *ui;
};

}
}

#endif // RESULTSVIEW_H
