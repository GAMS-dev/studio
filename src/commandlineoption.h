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

    QString getCurrentCommandLineOption() const;
    void setCurrentCommandLineOption(QString text);

    QString getCurrentOption() const;

    bool isValidated() const;
    void validated(bool value);

signals:
    void optionRunChanged();
    void optionRunWithParameterChanged(QString parameter);

public slots:
    void updateCurrentOption(QString text);
    void validateChangedOption(QString text);

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;

private:
    QString mCurrentOption;
    bool mValidated;
    CommandLineTokenizer* mCommandLineTokenizer;
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEOPTION_H

