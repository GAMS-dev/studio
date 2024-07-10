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
#include "navigatorcontent.h"

namespace gams {
namespace studio {

NavigatorContent::NavigatorContent() { }

// known files
NavigatorContent::NavigatorContent(FileMeta* file, const QString &additionalText) {
    mFileMeta = file;
    if (file) {
        mFileInfo = QFileInfo(file->location());
        mText = mFileInfo.fileName();
    }
    mAdditionalInfo = additionalText;
}

// unknown files
NavigatorContent::NavigatorContent(const QFileInfo &file, const QString &additionalText) {
    mFileInfo = file;
    mText = file.fileName();
    mAdditionalInfo = additionalText;
}

// help content
NavigatorContent::NavigatorContent(const QString &txt, const QString &additionalText,
                                   const QString &prefix, FileMeta* currentFile) {
    mText = txt;
    mAdditionalInfo = additionalText;
    mInsertPrefix = prefix;
    mFileMeta = currentFile;
}

// quick actions
NavigatorContent::NavigatorContent(const QString &txt, std::function<void ()> function)
    : mText(txt)
    , mFunction(std::move(function))
{

}

bool NavigatorContent::operator==(const NavigatorContent &other) const
{
    return fileMeta() == other.fileMeta() &&
           mText == other.text();
}

bool NavigatorContent::isValid()
{
    return !mAdditionalInfo.isEmpty() || mFunction.target<void>();
}

FileMeta *NavigatorContent::fileMeta() const
{
    return mFileMeta;
}

QFileInfo NavigatorContent::fileInfo() const
{
    return mFileInfo;
}

QString NavigatorContent::text() const
{
    return mText.isEmpty() ? fileInfo().fileName() : mText;
}

QString NavigatorContent::additionalInfo() const
{
    return mAdditionalInfo;
}

QString NavigatorContent::prefix() const
{
    return mInsertPrefix;
}

void NavigatorContent::executeQuickAction() const
{
    mFunction();
}

}
}
