/*
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
 */
#include "navigatorcontent.h"

namespace gams {
namespace studio {

NavigatorContent::NavigatorContent() { }

// known files
NavigatorContent::NavigatorContent(FileMeta* file, QString additionalText) {
    mFileMeta = file;
    if (file) mFileInfo = QFileInfo(file->location());
    mAdditionalInfo = additionalText;
}

// unknown files
NavigatorContent::NavigatorContent(QFileInfo file, QString additionalText) {
    mFileInfo = file;
    mAdditionalInfo = additionalText;
}

// help content
NavigatorContent::NavigatorContent(QString txt, QString additionalText,
                                   QString prefix, FileMeta* currentFile) {
    mText = txt;
    mAdditionalInfo = additionalText;
    mInsertPrefix = prefix;
    mFileMeta = currentFile;
}

// quick actions
NavigatorContent::NavigatorContent(QString txt, std::function<void()> function) {
    mText = txt;
    mFunction = function;
}

bool NavigatorContent::isValid()
{
    return !mAdditionalInfo.isEmpty() || mFunction.target<void>();
}

FileMeta *NavigatorContent::GetFileMeta()
{
    return mFileMeta;
}

QFileInfo NavigatorContent::FileInfo()
{
    return mFileInfo;
}

QString NavigatorContent::Text()
{
    return mText;
}

QString NavigatorContent::AdditionalInfo()
{
    return mAdditionalInfo;
}

QString NavigatorContent::Prefix()
{
    return mInsertPrefix;
}

void NavigatorContent::ExecuteQuickAction()
{
    mFunction();
}

}
}
