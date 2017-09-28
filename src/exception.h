#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QtCore>

namespace gams {
namespace studio {

class Exception : public QException
{
public:
    Exception() { mStream = new QTextStream(&mBuffer); }

    ~Exception() throw() {
        qDebug() << mBuffer;
        delete mStream;
    }

    void raise() const { throw *this; }
    Exception *clone() const { return new Exception(*this); }

    template <typename T> Exception& operator<<(const T& value) {
        (*mStream) << value;
        return *this;
    }

protected:
    QString mBuffer;
    QTextStream *mStream;
};

class FatalException : public Exception
{
public:
    FatalException() {}
    void raise() const { throw *this; }
    FatalException *clone() const { return new FatalException(*this); }
};

} // namespace studio
} // namespace gams


#ifdef QT_DEBUG
#  ifdef __GNUC__
#    define EXCEPT() gams::studio::Exception() << '[' <<__PRETTY_FUNCTION__ << __FILE__ << __LINE__ << ']'
#    define FATAL() gams::studio::FatalException() << '[' <<__PRETTY_FUNCTION__ << __FILE__ << __LINE__ << ']'
#  else
#    define EXCEPT() gams::studio::Exception() << '[' <<__PRETTY_FUNCTION__ << __FILE__ << __LINE__ << ']'
#    define FATAL() gams::studio::FatalException() << '[' <<__FUNCSIG__ << __FILE__ << __LINE__ << ']'
#  endif
#else
#  define FATAL() gams::studio::Exception()
#endif

#endif // EXCEPTION_H
