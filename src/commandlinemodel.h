#ifndef COMMANDLINEMODEL_H
#define COMMANDLINEMODEL_H

#include <QtWidgets>

namespace gams {
namespace studio {

class CommandLineModel : public QObject
{
    Q_OBJECT

public:
    CommandLineModel(QObject* parent, int initialHistorySize = 20);
    CommandLineModel(QMap<QString, QStringList> map);
    ~CommandLineModel();

    QStringList getHistoryFor(QString context);
    void setHistory(QString context, QStringList history);

    QMap<QString, QStringList> allHistory() const;
    void setAllHistory(QMap<QString, QStringList> opts);

    int getHistorySize() const;
    void setHistorySize(int historySize);
    QMap<QString, QStringList> allOptions();

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
