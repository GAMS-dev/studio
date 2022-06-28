#ifndef GAMS_STUDIO_FILTERLINEEDIT_H
#define GAMS_STUDIO_FILTERLINEEDIT_H

#include <QLineEdit>

namespace gams {
namespace studio {



class FilterLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    enum FilterLineEditFlag {
        foNone   = 0x00,
        foClear  = 0x01,
        foExact  = 0x02,
        foRegEx  = 0x04,
        foColumn = 0x08,
    };
    Q_DECLARE_FLAGS(FilterLineEditFlags, FilterLineEditFlag)
    Q_FLAG(FilterLineEditFlags)

public:
    explicit FilterLineEdit(QWidget *parent = nullptr);
    explicit FilterLineEdit(const QString &contents, QWidget *parent = nullptr);
    const QRegExp &regExp() const;
    void setOptionState(FilterLineEditFlag option, int state);
    void setKeyColumn(int column);
    void hideOptions(FilterLineEditFlags options);
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
    QToolButton *button(FilterLineEditFlag option);

private:
    QToolButton *mClearButton = nullptr;
    QToolButton *mExactButton = nullptr;
    QToolButton *mRegExButton = nullptr;
    QToolButton *mAllColButton = nullptr;
    QRegExp mRegExp;
    bool mCanClear = true;
    int mKeyColumn = -1;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_FILTERLINEEDIT_H
