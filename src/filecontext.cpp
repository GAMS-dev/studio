/*
 * This file is part of the GAMS IDE project.
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
#include "filecontext.h"
#include "filegroupcontext.h"

namespace gams {
namespace ide {

FileContext::FileContext(FileGroupContext *parent, int id, QString name, QString location, bool isGist)
    : FileSystemContext(parent, id, name, location, isGist)
{
    mActive = true;
}

CrudState FileContext::crudState() const
{
    return mCrudState;
}

void FileContext::saved()
{
    if (mCrudState != CrudState::eRead) {
        mCrudState = CrudState::eRead;
        emit nameChanged(mId, name());
    }
}

QString FileContext::codec() const
{
    return mCodec;
}

void FileContext::setCodec(const QString& codec)
{
    // TODO(JM) changing the codec must trigger conversion (not necessarily HERE)
    mCodec = codec;
}

const QString FileContext::name()
{
    return mName + (mCrudState==CrudState::eUpdate ? "*" : "");
}

void FileContext::textChanged()
{
    if (mCrudState != CrudState::eUpdate) {
        mCrudState = CrudState::eUpdate;
        emit nameChanged(mId, name());
    }
}

} // namespace ide
} // namespace gams
