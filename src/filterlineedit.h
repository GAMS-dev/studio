#ifndef GAMS_STUDIO_FILTERLINEEDIT_H
#define GAMS_STUDIO_FILTERLINEEDIT_H

#include <QLineEdit>
#include <QPushButton>

namespace gams {
namespace studio {


class MiniButton : public QPushButton
{
    Q_OBJECT
public:
    MiniButton(QWidget *parent = nullptr): QPushButton(parent) {}
    virtual ~MiniButton() override {}
    QSize sizeHint() const override;
};


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
    QAbstractButton *createButton(const QStringList &iconPaths, const QStringList &toolTips);
    int nextButtonState(QAbstractButton *button, int forceState = -1);
    int buttonState(QAbstractButton *button);
    void updateTextMargins();
    QAbstractButton *button(FilterLineEditFlag option);

private:
    QAbstractButton *mClearButton = nullptr;
    QAbstractButton *mExactButton = nullptr;
    QAbstractButton *mRegExButton = nullptr;
    QAbstractButton *mAllColButton = nullptr;
    QRegExp mRegExp;
    bool mCanClear = true;
    int mKeyColumn = -1;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_FILTERLINEEDIT_H
