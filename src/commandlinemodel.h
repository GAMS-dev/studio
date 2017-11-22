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

    QStringList getHistoryFor(QString context);
    void setHistory(QString context, QStringList history);

    QMap<QString, QStringList> getAllHistory() const;

    int getHistorySize() const;
    void setHistorySize(int historySize);

public slots:
    void addIntoCurrentContextHistory(QString option);

private:
    QMap<QString, QStringList> mHistory;
    QString mCurrentContext;
    int mHistorySize;

    void setContext(QString context);
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEMODEL_H
