#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>
#include <QRegularExpression>
#include <QTextDocument>

namespace gams {
namespace studio {
namespace find {

namespace Ui {
class FindWidget;
}

class FindWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindWidget(QWidget *parent = nullptr);
    ~FindWidget();
    bool active() const;
    void setActive(bool newActive);
    void setFindText(const QString &text);
    void setReadonly(bool readonly = true);
    QRegularExpression termRexEx();
    QTextDocument::FindFlags findFlags(bool backwards = false);
    void triggerFind(bool backwards = false);

signals:
    void find(const QRegularExpression &rex, QTextDocument::FindFlags options = QTextDocument::FindFlags());

protected:
    void focusInEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void on_bClose_clicked();

private:
    Ui::FindWidget *ui;
    bool mActive = false;
};

} // namespace find
} // namespace studio
} // namespace gams
#endif // FINDWIDGET_H
