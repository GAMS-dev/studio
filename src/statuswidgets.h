#ifndef STATUSWIDGETS_H
#define STATUSWIDGETS_H

#include <QObject>
#include <QPoint>

class QStatusBar;
class QMainWindow;
class QLabel;

namespace gams {
namespace studio {

enum class EditMode {Readonly, Insert, Overwrite};

class StatusWidgets : public QObject
{
    Q_OBJECT
public:
    explicit StatusWidgets(QMainWindow *parent);
    void setFileName(const QString &fileName);
    void setEncoding(int encodingMib);
    void setLineCount(int lines);
    void setEditMode(EditMode mode);
    void setPosAndAnchor(QPoint pos = QPoint(), QPoint anchor = QPoint());

signals:

public slots:

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
