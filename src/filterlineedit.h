#ifndef GAMS_STUDIO_FILTERLINEEDIT_H
#define GAMS_STUDIO_FILTERLINEEDIT_H

#include <QLineEdit>

namespace gams {
namespace studio {

class FilterLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit FilterLineEdit(QWidget *parent = nullptr);
    explicit FilterLineEdit(const QString &contents, QWidget *parent = nullptr);
    const QRegExp &regExp() const;
    void hideColumnButton(bool allColumns);
    void setKeyColumn(int column);
    int effectiveKeyColumn();

signals:
    void regExpChanged(const QRegExp &regExp);
    void columnScopeChanged();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void init();
    void updateRegExp();
    QToolButton *createButton(const QStringList &iconPaths, const QStringList &toolTips);
    int nextButtonState(QToolButton *button, int forceState = -1);
    int buttonState(QToolButton *button);
    void updateTextMargins();

private:
    QToolButton *mClearButton = nullptr;
    QToolButton *mExactButton = nullptr;
    QToolButton *mRegExButton = nullptr;
    QToolButton *mAllColButton = nullptr;
    QRegExp mRegExp;
    int mKeyColumn = -1;

};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_FILTERLINEEDIT_H
