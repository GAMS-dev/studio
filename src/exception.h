/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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

#include <QtCore>

namespace gams {
namespace studio {

class Exception : public QException
{
public:
    Exception();
    Exception(const Exception &exception);
    ~Exception();

    void raise() const { throw *this; }
    Exception *clone() const { return new Exception(*this); }

    template <typename T> Exception& operator<<(const T& value) {
        (*mStream) << value;
        (*mStream).flush();
        return *this;
    }
    const char* what();

protected:
    QByteArray mBuffer;
    QTextStream *mStream;
};

class FatalException : public Exception
{
public:
    FatalException() {}
    void raise() const { throw *this; }
    FatalException *clone() const;
};

} // namespace studio
} // namespace gams


#ifdef QT_DEBUG
#  ifdef __GNUC__
#    define EXCEPT() throw gams::studio::Exception() << '<' <<__PRETTY_FUNCTION__ << __FILE__ << __LINE__ << "> "
#    define FATAL() throw gams::studio::FatalException() << '<' <<__PRETTY_FUNCTION__ << __FILE__ << __LINE__ << "> "
#  else
#    define EXCEPT() throw gams::studio::Exception() << '<' <<__FUNCSIG__ << __FILE__ << __LINE__ << "> "
#    define FATAL() throw gams::studio::FatalException() << '<' <<__FUNCSIG__ << __FILE__ << __LINE__ << "> "
#  endif
#else
#  define EXCEPT() throw gams::studio::Exception()
#  define FATAL() throw gams::studio::FatalException()
#endif

#endif // EXCEPTION_H
