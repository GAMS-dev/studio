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
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QException>
#include <QTextStream>
#include <QDebug>

namespace gams {
namespace studio {

class Exception : public QException
{
public:
    Exception();
    Exception(const Exception &other);
    ~Exception();
    Exception &operator =(const Exception& other);

    void raise() const { throw *this; }
    Exception *clone() const { return new Exception(*this); }

    Exception& operator<<(const char value) {
        if (mInfoStart < 0) {
            (*mStream) << '[';
            mInfoStart++;
        }
        if (mInfoStart == 0 && value=='\t') {
            (*mStream) << "] ";
            (*mStream).flush();
            mInfoStart = mBuffer.length();
        } else {
            (*mStream) << value;
            (*mStream).flush();
        }
        return *this;
    }

    template <typename T> Exception& operator<<(const T& value) {
        if (mInfoStart < 0) {
            (*mStream) << '[';
            mInfoStart = 0;
        }
        (*mStream) << value;
        (*mStream).flush();
        return *this;
    }
    const char* what() const noexcept;
    virtual QByteArray where();

protected:
    QByteArray mBuffer;
    QTextStream *mStream;
    int mInfoStart = -1;
};

class FatalException : public Exception
{
public:
    FatalException();
    FatalException &operator =(const FatalException& other);
    template <typename T> FatalException& operator<<(const T& value) {
        Exception::operator <<(value);
        return *this;
    }
    FatalException(const FatalException &exception) : Exception(exception) {}
    void raise() const { throw *this; }
    FatalException *clone() const;
};

} // namespace studio
} // namespace gams


#ifdef QT_DEBUG
#  ifdef __GNUC__
#    define EXCEPT() throw gams::studio::Exception() << __PRETTY_FUNCTION__ << __FILE__ << __LINE__ << '\t'
#    define FATAL() throw gams::studio::FatalException() << __PRETTY_FUNCTION__ << __FILE__ << __LINE__ << '\t'
#  else
#    define EXCEPT() throw gams::studio::Exception() << __FUNCSIG__ << __FILE__ << __LINE__ << '\t'
#    define FATAL() throw gams::studio::FatalException() <<__FUNCSIG__ << __FILE__ << __LINE__ << '\t'
#  endif
#else
#  define EXCEPT() throw gams::studio::Exception()
#  define FATAL() throw gams::studio::FatalException()
#endif

#endif // EXCEPTION_H
