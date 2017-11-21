#ifndef COMMANDLINEMODEL_H
#define COMMANDLINEMODEL_H

#include <QtWidgets>

namespace gams {
namespace studio {

class CommandLineModel : public QWidget
{
    Q_OBJECT
public:
    CommandLineModel(QWidget* parent=0);
    ~CommandLineModel();

    QStringList getOptionsFor(QString context);
    void setContext(QString context);

public slots:
    void addOptionIntoCurrentContext(QString option);

private:
    QString mCurrentContext;
    QMap<QString, QStringList> mOptions;

};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEMODEL_H
