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


class AmountLabel: public QLabel
{
    Q_OBJECT
    qreal mLoadAmount = 1.0;
    QString mLoadingText;
    QString mBaseText;
public:
    AmountLabel(QWidget *parent) : QLabel(parent) {}
    explicit AmountLabel(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QLabel(parent, f) {}
    explicit AmountLabel(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QLabel(text, parent, f) { setBaseText(text); }
    qreal getAmount() const { return mLoadAmount; }
    void setAmount(qreal value);
    void setBaseText(const QString &text);
    void setLoadingText(const QString &loadingText);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class StatusWidgets : public QObject
{
    Q_OBJECT
public:
    enum class EditMode {Readonly, Insert, Overwrite};
    Q_ENUM(EditMode)

public:
    explicit StatusWidgets(QMainWindow *parent);
    void setFileName(const QString &fileName);
    void setEncoding(int encodingMib);
    void setLineCount(int lines);
    void setLoadAmount(qreal amount);
    void setEditMode(EditMode mode);
    void setPosAndAnchor(QPoint pos = QPoint(), QPoint anchor = QPoint());
    void setLoadingText(const QString &loadingText);

private:
    QStatusBar* mStatusBar;
    QLabel* mEditMode = nullptr;
    QLabel* mEditEncode = nullptr;
    QLabel* mEditPosAnsSel = nullptr;
    QLabel* mEditLines = nullptr;
    AmountLabel* mFileName = nullptr;
    QLabel* mProcessInfo = nullptr;
    qreal mLoadAmount = 1.0;

};

} // namespace Studio
} // namespace gams

#endif // STATUSWIDGETS_H
