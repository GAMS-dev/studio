#ifndef COMMANDLINEOPTION_H
#define COMMANDLINEOPTION_H

#include <QtCore>
#include<QComboBox>

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
    void activateRunWithCurrentOption(QString text);
    void validateChangedOption(QString text);

private:
    QString mCurrentOption;
};

#endif // COMMANDLINEOPTION_H
