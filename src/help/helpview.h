#ifndef HELPVIEW_H
#define HELPVIEW_H

#include <QWebEngineView>

namespace gams {
namespace studio {

class HelpPage;

class HelpView : public QWebEngineView
{
   Q_OBJECT

public:
    HelpView(QWidget *parent = nullptr);
    void setPage(QWebEnginePage *page);
};

} // namespace studio
} // namespace gams

#endif // HELPVIEW_H
