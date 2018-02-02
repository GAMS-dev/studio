#ifndef COMMANDLINEOPTION_H
#define COMMANDLINEOPTION_H

#include <QtCore>
#include<QComboBox>
#include "commandlinetokenizer.h"

namespace gams {
namespace studio {

class CommandLineOption : public QComboBox
{
    Q_OBJECT

public:
    CommandLineOption(bool validateFlag, QWidget* parent);
    ~CommandLineOption();

    QString getCurrentOption() const;

    bool isValidated() const;
    void validated(bool value);

    QString getCurrentContext() const;
    void setCurrentContext(const QString &currentContext);

    void resetCurrentValue();

signals:
    void optionRunChanged();
    void commandLineOptionChanged(QLineEdit* lineEdit, const QString &commandLineStr);

public slots:
    void validateChangedOption(const QString &text);

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;

private:
    QString mCurrentOption;
    QString mCurrentContext;
    int mCurrentIndex;
    bool mValidated;
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEOPTION_H

