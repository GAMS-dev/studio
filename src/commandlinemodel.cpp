#include "commandlinemodel.h"

namespace gams {
namespace studio {

CommandLineModel::CommandLineModel(int initialHistorySize):
    mHistorySize(initialHistorySize)
{
}

CommandLineModel::~CommandLineModel()
{
    mHistory.clear();
}

void CommandLineModel::setHistory(QString context, QStringList history)
{
    mHistory[context] = history;
}

void CommandLineModel::addIntoCurrentContextHistory(QString option)
{
    if (option.simplified().isEmpty())
        return;

    if (mHistory.contains(mCurrentContext)) {
        QStringList list = mHistory[mCurrentContext].filter(option.simplified());
        if (list.size() > 0) {
            mHistory[mCurrentContext].removeOne(option.simplified());
        }

        if (mHistory[mCurrentContext].size() >= mHistorySize) {
            mHistory[mCurrentContext].removeFirst();
        }
        mHistory[mCurrentContext].append(option.simplified());
    }
}

int CommandLineModel::getHistorySize() const
{
    return mHistorySize;
}

void CommandLineModel::setHistorySize(int historySize)
{
    mHistorySize = historySize;
}

QMap<QString, QStringList> CommandLineModel::getAllHistory() const
{
    return mHistory;
}

QStringList CommandLineModel::getHistoryFor(QString context)
{
    setContext(context);
    return mHistory[mCurrentContext];
}

void CommandLineModel::setContext(QString context)
{
    mCurrentContext = context;
    if (!mHistory.contains(mCurrentContext)) {
        QStringList strList;
        mHistory[mCurrentContext] = strList;
    }
}

} // namespace studio
} // namespace gams
