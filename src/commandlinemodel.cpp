#include "commandlinemodel.h"

namespace gams {
namespace studio {

CommandLineModel::CommandLineModel(QWidget* parent):
    QWidget(parent)
{
}

CommandLineModel::CommandLineModel(QMap<QString, QStringList> map)
{
    setAllOptions(map);
}

CommandLineModel::~CommandLineModel()
{
    mOptions.clear();
}

void CommandLineModel::setContext(QString context)
{
    mCurrentContext = context;
    if (!mOptions.contains(mCurrentContext)) {
        QStringList strList;
        mOptions[mCurrentContext] = strList;
    }
}

void CommandLineModel::addOptionIntoCurrentContext(QString option)
{
    if (mOptions.contains(mCurrentContext)) {
        QStringList list = mOptions[mCurrentContext].filter(option);
        if (list.size()>0) {
            mOptions[mCurrentContext].removeOne(option);
        }
        mOptions[mCurrentContext].append(option);
    }
}

void CommandLineModel::setAllOptions(QMap<QString, QStringList> opts)
{
    mOptions = opts;
}

QMap<QString, QStringList> CommandLineModel::allOptions()
{
    return mOptions;
}

QStringList CommandLineModel::getOptionsFor(QString context)
{
    setContext(context);
    return mOptions[mCurrentContext];
}

} // namespace studio
} // namespace gams
