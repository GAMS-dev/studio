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
#include "dynamicfile.h"
#include "logger.h"
#include <QMutexLocker>
#include <QDir>

namespace gams {
namespace studio {

DynamicFile::DynamicFile(const QString &fileName, int backups, QObject *parent): QObject(parent), mBackups(backups)
{
    mFile.setFileName(fileName);
    if (mFile.exists()) runBackupCircle();
}

DynamicFile::~DynamicFile()
{
    closeFile();
}

void DynamicFile::appendLine(const QString &line)
{
    if (!mFile.isOpen())
        openFile();
    QMutexLocker locker(&mMutex);
    mEnd = mFile.size();
    if (mFile.isOpen()) {
        mFile.write(line.toUtf8());
        mFile.write("\n");
    }
}

void DynamicFile::confirmLastLine()
{
    if (!mFile.isOpen())
        openFile();
    QMutexLocker locker(&mMutex);
    if (mFile.isOpen()) {
        mEnd = mFile.size();
    }
}

void DynamicFile::closeFile()
{
    QMutexLocker locker(&mMutex);
    if (mFile.isOpen()) {
        mFile.flush();
        mFile.close();
        runBackupCircle();
    }
}

void DynamicFile::openFile()
{
    QMutexLocker locker(&mMutex);
    if (!mFile.isOpen()) {
        mFile.open(QFile::WriteOnly);
        mFile.seek(mEnd);
    }
}

void DynamicFile::runBackupCircle()
{
    QStringList names;

    QFileInfo fi(mFile);
    QString suffix = fi.suffix();
    names << mFile.fileName();
    // if filename has a temp-marker add non-temp filename to backup-circle
    if (suffix.contains('~')) names.prepend(mFile.fileName().remove('~'));

    // add all backup filenames until one doesn't exist
    for (int i = 1; i <= mBackups; ++i) {
        names.prepend(mFile.fileName().remove('~') + "~" + QString::number(i));
        if (!QFile(names.first()).exists()) break;
    }
    // last backup will be overwritten - if it exists, delete it
    QFile file(names.first());
    if (file.exists()) file.remove();

    QString destName;
    for (const QString &sourceName: std::as_const(names)) {
        if (!destName.isEmpty()) {
            file.setFileName(sourceName);
            if (file.exists()) file.rename(destName);
        }
        destName = sourceName;
    }
}

} // namespace studio
} // namespace gams
