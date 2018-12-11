#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H

#include <QTreeView>

namespace gams {
namespace studio {

class ProjectTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit ProjectTreeView(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void fixFocus();

public slots:
    void selectAll() override;
};

} // namespace studio
} // namespace gams

#endif // PROJECTTREEVIEW_H
