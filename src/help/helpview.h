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
    void setCurrentHoveredLink(const QString &url);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;

private:
    QString mCurrentHovered;
};

} // namespace studio
} // namespace gams

#endif // HELPVIEW_H
