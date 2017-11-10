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

} // namespace studio
} // namespace gams

#ifdef QT_DEBUG
#  ifdef __GNUC__
//#    define TRACE() gams::studio::Tracer  _GamsTracer_(__PRETTY_FUNCTION__)
#    define TRACE() gams::studio::Tracer  _GamsTracer_(__FUNCTION__)
#    define DEB() gams::studio::Logger() << Logger::indent()
#  else
#    define TRACE() gams::studio::Tracer _GamsTracer_(__FUNCSIG__)
#    define DEB() gams::studio::Logger() << Logger::indent()
#  endif
#else
#  define TRACE() gams::studio::Tracer _GamsTracer_()
#  define DEB()  gams::studio::Logger() << Logger::indent()
#endif

#endif // LOGGER_H
