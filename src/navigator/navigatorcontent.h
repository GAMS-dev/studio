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
#ifndef NAVIGATORCONTENT_H
#define NAVIGATORCONTENT_H

#include "file/filemeta.h"

namespace gams {
namespace studio {

class NavigatorContent
{
public:
    NavigatorContent();
    NavigatorContent(FileMeta* file, QString additionalText);
    NavigatorContent(QFileInfo file, QString additionalText);
    NavigatorContent(QString txt, QString additionalText, QString prefix, FileMeta* currentFile = nullptr);
    NavigatorContent(QString txt, std::function<void()> function);
    bool isValid();

    FileMeta *GetFileMeta();
    QFileInfo FileInfo();
    QString Text();
    QString AdditionalInfo();
    QString Prefix();
    void ExecuteQuickAction();

private:
    FileMeta* mFileMeta = nullptr;
    QFileInfo mFileInfo;
    QString mText;
    QString mAdditionalInfo;
    QString mInsertPrefix;
    std::function<void()> mFunction;
};

}
}
#endif // NAVIGATORCONTENT_H
