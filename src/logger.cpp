#include "logger.h"

namespace gams {
namespace studio {

Logger::Logger()
{
}

Logger::~Logger()
{
    if (mStream) {
        qDebug().noquote() << indent() << mBuffer;
        delete mStream;
    }
}

void Logger::incDepth()
{
    ++mDepth;
    mIndent = (mDepth>0) ? QString(mDepth*2, ' ') : QString("");
}

void Logger::decDepth()
{
    --mDepth;
    mIndent = (mDepth>0) ? QString(mDepth*2, ' ') : QString("");
}

const QString& Logger::indent()
{
    return mIndent;
}

int Logger::mDepth = 0;
QString Logger::mIndent("");

} // namespace studio
} // namespace gams
