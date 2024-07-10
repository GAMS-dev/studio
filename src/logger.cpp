/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "logger.h"

namespace gams {
namespace studio {

Logger::Logger(const char* file, int line, const char* func)
    : mFile(file), mLine(line), mFunc(func)
{}

Logger::~Logger()
{
    if (mStream) {
        QMessageLogger(mFile, mLine, mFunc).debug().noquote().nospace() << indent() << mBuffer;
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
