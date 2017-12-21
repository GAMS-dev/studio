#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>
#include <iostream>
#include <string>

namespace gams {
namespace studio {

class Logger
{
public:
    Logger();
    virtual ~Logger();

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
    QString mBuffer;
    QTextStream *mStream = nullptr;
private:
    static int mDepth;
    static QString mIndent;
};

class Tracer: public Logger
{
public:
    Tracer(QString functionName = QString()): mFunctionName(functionName) {
        QRegularExpression regex("[^\\_]*\\_\\_cdecl ([^\\(]+)"); // (\\_\\_ccdecl )([^\\(])+
        QRegularExpressionMatch match = regex.match(functionName);
        if (match.hasMatch())
            mFunctionName = match.captured(1)+"(...)";
        if (!mFunctionName.isEmpty())
            qDebug().noquote() << indent() << "IN " << mFunctionName;
        incDepth();
    }
    ~Tracer() {
        decDepth();
        if (!mFunctionName.isEmpty())
            qDebug().noquote() << indent() << "OUT" << mFunctionName;
    }
private:
    QString mFunctionName;
};

class TimeTracer: public Logger
{
public:
    TimeTracer(QString functionName = QString()): mFunctionName(functionName) {
        mSec = QDateTime::currentMSecsSinceEpoch();
        QRegularExpression regex("[^\\_]*\\_\\_cdecl ([^\\(]+)"); // (\\_\\_ccdecl )([^\\(])+
        QRegularExpressionMatch match = regex.match(functionName);
        if (match.hasMatch())
            mFunctionName = match.captured(1)+"(...)";
        if (!mFunctionName.isEmpty())
            qDebug().noquote() << indent() << "IN " << mFunctionName;
        incDepth();
    }
    ~TimeTracer() {
        decDepth();
        if (!mFunctionName.isEmpty())
            qDebug().noquote() << indent() << "OUT " << timeString() << " " << mFunctionName;
    }
    QString timeString() {
        int elapsed = QDateTime::currentMSecsSinceEpoch() - mSec;
        return QString::number(elapsed); //  tim.toString("m:ss:zzz");
    }
private:
    QString mFunctionName;
    qint64 mSec = 0;
};

} // namespace studio
} // namespace gams

#ifdef QT_DEBUG
#  ifdef __GNUC__
//#    define TRACE() gams::studio::Tracer  _GamsTracer_(__PRETTY_FUNCTION__)
#    define TRACETIME() gams::studio::TimeTracer  _GamsTracer_(__FUNCTION__)
#    define TRACE() gams::studio::Tracer  _GamsTracer_(__FUNCTION__)
#    define DEB() gams::studio::Logger() << Logger::indent()
#  else
#    define TRACETIME() gams::studio::TimeTracer _GamsTimeTracer_(__FUNCSIG__)
#    define TRACE() gams::studio::Tracer _GamsTracer_(__FUNCSIG__)
#    define DEB() gams::studio::Logger() << Logger::indent()
#  endif
#else
#  define TRACETIME() gams::studio::TimeTracer _GamsTracer_()
#  define TRACE() gams::studio::Tracer _GamsTracer_()
#  define DEB()  gams::studio::Logger() << Logger::indent()
#endif

#endif // LOGGER_H
