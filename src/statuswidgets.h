#ifndef STATUSWIDGETS_H
#define STATUSWIDGETS_H

#include <QObject>
#include <QPoint>
#include <QLabel>

class QStatusBar;
class QMainWindow;
class QLabel;

namespace gams {
namespace studio {

enum class EditMode {Readonly, Insert, Overwrite};

class AmountLabel: public QLabel
{
    Q_OBJECT
    int amount = 0;
public:
    AmountLabel(QWidget *parent) : QLabel(parent) {}
    explicit AmountLabel(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QLabel(parent, f) {}
    explicit AmountLabel(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QLabel(text, parent, f) {}
    int getAmount() const { return amount; }
    void setAmount(int value) { amount = value; }
protected:
//    void paintEvent(QPaintEvent *event) override;
};

class StatusWidgets : public QObject
{
    Q_OBJECT

public:
    explicit StatusWidgets(QMainWindow *parent);
    void setFileName(const QString &fileName);
    void setEncoding(int encodingMib);
    void setLineCount(int lines);
    void setLoadAmount(qreal amount);
    void setEditMode(EditMode mode);
    void setPosAndAnchor(QPoint pos = QPoint(), QPoint anchor = QPoint());

private:
    QStatusBar* mStatusBar;
    QLabel* mEditMode = nullptr;
    QLabel* mEditEncode = nullptr;
    QLabel* mEditPosAnsSel = nullptr;
    QLabel* mEditLines = nullptr;
    QLabel* mFileName = nullptr;
    QLabel* mProcessInfo = nullptr;
};

} // namespace Studio
} // namespace gams

#endif // STATUSWIDGETS_H
