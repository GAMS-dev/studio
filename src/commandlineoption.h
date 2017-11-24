#ifndef COMMANDLINEOPTION_H
#define COMMANDLINEOPTION_H

#include <QtCore>
#include<QComboBox>

namespace gams {
namespace studio {

class CommandLineOption : public QComboBox
{
    Q_OBJECT

public:
    CommandLineOption(QWidget* parent);

    QString getCurrentCommandLineOption() const;
    void setCurrentCommandLineOption(QString text);

    QString getCurrentOption() const;

signals:
    void runWithChangedOption(QString text);

public slots:
    void updateCurrentOption(QString text);
    void validateChangedOption(QString text);

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;

private:
    QString mCurrentOption;
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEOPTION_H

