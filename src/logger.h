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
#ifndef LOGGER_H
#define LOGGER_H

#include <QRect>
#include <QDebug>
#include <QTextStream>
#include <QString>
#include <QDateTime>
#include <QRegularExpression>
#include <iostream>
#include <string>

namespace gams {
namespace studio {

class Logger
{
public:
    Logger(const char *file, int line, const char *func);
    virtual ~Logger();

    Logger& operator<<(const QRect& value) {
        if (!mStream)
            mStream = new QTextStream(&mBuffer);
        (*mStream) << "Rect(" << value.x() << "," << value.y() << "," << value.width() << "," << value.height() << ")";
        return *this;
    }

    Logger& operator<<(const QPoint& value) {
        if (!mStream)
            mStream = new QTextStream(&mBuffer);
        (*mStream) << "Point(" << value.x() << "," << value.y() << ")";
        return *this;
    }

    Logger& operator<<(const QSize& value) {
        if (!mStream)
            mStream = new QTextStream(&mBuffer);
        (*mStream) << "Point(" << value.width() << "," << value.height() << ")";
        return *this;
    }

    template <typename T> Logger& operator<<(const T& value) {
        if (!mStream)
            mStream = new QTextStream(&mBuffer);
        (*mStream) << value;
        return *this;
    }

    static void incDepth();
    static void decDepth();
    static const QString& indent();

protected:
    const char *mFile;
    int mLine;
    const char *mFunc;
    QString mBuffer;
    QTextStream *mStream = nullptr;
private:
    static int mDepth;
    static QString mIndent;
};

class Tracer: public Logger
{
public:
    Tracer(const char *file, int line, const char *func): Logger(file, line, func) {
        mFunctionName = func;
        int a = qMax(0, mFunctionName.indexOf(" __cdecl "));
        a = qMax(0, mFunctionName.indexOf("gams::studio::", a+9));
        if (a > 0) a += 14;
        int b = mFunctionName.indexOf('(', a);
        if (a > 0 && a < b) mFunctionName = mFunctionName.mid(a, b-a)+"(..)";
        if (!mFunctionName.isEmpty()) {
                             // get QtCreator to generate a link to the source file
            mFunctionName += QString(" [Object::in %1:%2]").arg(file).arg(line);
            QMessageLogger(mFile, mLine, "").debug().noquote().nospace() << indent() << "IN  "  << mFunctionName;
        }
        incDepth();
    }
    ~Tracer() override {
        decDepth();
        if (!mFunctionName.isEmpty())
            QMessageLogger(mFile, mLine, "").debug().noquote().nospace() << indent() << "OUT " << mFunctionName;
    }
private:
    QString mFunctionName;
};

class TimeTracer: public Logger
{
public:
    TimeTracer(const char *file, int line, const char *func): Logger(file, line, func) {
        mSec = QDateTime::currentMSecsSinceEpoch();
        mFunctionName = func;
        int a = qMax(0, mFunctionName.indexOf(" __cdecl "));
        a = qMax(0, mFunctionName.indexOf("gams::studio::", a+9));
        if (a > 0) a += 14;
        int b = mFunctionName.indexOf('(', a);
        if (a > 0 && a < b) mFunctionName = mFunctionName.mid(a, b-a)+"(..)";
        if (!mFunctionName.isEmpty())
            QMessageLogger(mFile, mLine, "").debug().noquote().nospace() << indent() << "IN  " << mFunctionName;
        incDepth();
    }
    ~TimeTracer() override {
        decDepth();
        if (!mFunctionName.isEmpty())
            *this << "OUT " << timeString() << " ms " << mFunctionName;
    }
    QString timeString() {
        qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - mSec;
        return QString::number(elapsed); //  tim.toString("m:ss:zzz");
    }
private:
    QString mFunctionName;
    qint64 mSec = 0;
};

} // namespace studio
} // namespace gams

#define TRACETIME() gams::studio::TimeTracer _GamsTimeTracer_(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC);
#define PEEKTIME() gams::studio::Logger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC) << Logger::indent() << _GamsTimeTracer_.timeString() << " ms "
#define TRACE() gams::studio::Tracer _GamsTracer_(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC);
#define DEB() gams::studio::Logger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC)

#endif // LOGGER_H
