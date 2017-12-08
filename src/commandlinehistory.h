#ifndef COMMANDLINEHISTORY_H
#define COMMANDLINEHISTORY_H

#include <QtWidgets>

namespace gams {
namespace studio {

class CommandLineHistory : public QObject
{
    Q_OBJECT

public:
    static const int MAX_HISTORY_SIZE = 20;

    CommandLineHistory(QObject* parent, int initialHistorySize = MAX_HISTORY_SIZE);
    CommandLineHistory(QMap<QString, QStringList> map);
    ~CommandLineHistory();

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

#endif // COMMANDLINEHISTORY_H
