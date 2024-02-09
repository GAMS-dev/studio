/*
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
 */
#ifndef NAVIGATORCONTENT_H
#define NAVIGATORCONTENT_H

#include <QObject>
#include "file/filemeta.h"

namespace gams {
namespace studio {

class NavigatorContent
{
public:
    NavigatorContent();
    NavigatorContent(FileMeta* file, const QString &additionalText);
    NavigatorContent(const QFileInfo &file, const QString &additionalText);
    NavigatorContent(const QString &txt, const QString &additionalText, const QString &prefix, FileMeta* currentFile = nullptr);
    NavigatorContent(const QString &txt, std::function<void ()> function);

    bool operator==(const NavigatorContent &other) const;

    bool isValid();

    FileMeta *fileMeta() const;
    QFileInfo fileInfo() const;
    QString text() const;
    QString additionalInfo() const;
    QString prefix() const;
    void executeQuickAction() const;

private:
    FileMeta* mFileMeta = nullptr;
    QFileInfo mFileInfo;
    QString mText;
    QString mAdditionalInfo;
    QString mInsertPrefix;
    std::function<void()> mFunction;
};

inline uint qHash(const NavigatorContent &key, uint seed){
    return qHash(key.text(), seed) ^ qHash(key.fileMeta(), seed+1);
}


}
}
#endif // NAVIGATORCONTENT_H
