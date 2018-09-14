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

signals:

public slots:

protected:
    void focusOutEvent(QFocusEvent *event) override;
};

} // namespace studio
} // namespace gams

#endif // PROJECTTREEVIEW_H
