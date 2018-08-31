/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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

DynamicFile::DynamicFile(QString fileName, int backups, QObject *parent): QObject(parent)
{
    mFile.setFileName(QDir::toNativeSeparators(fileName));
    if (mFile.exists()) {
        DEB() << "handle existing destination file " << fileName;
        handleExisting(backups);
    }
    mCloseTimer.setSingleShot(true);
    mCloseTimer.setInterval(1000);
    connect(&mCloseTimer, &QTimer::timeout, this, &DynamicFile::closeFile);
}

DynamicFile::~DynamicFile()
{
    closeFile();
}

void DynamicFile::appendLine(QString line)
{
    if (!mFile.isOpen())
        openFile();
    QMutexLocker locker(&mMutex);
    if (mFile.isOpen()) {
        mFile.write(line.toUtf8());
        mFile.write("\n");
        mCloseTimer.start();
    }
}

void DynamicFile::closeFile()
{
    QMutexLocker locker(&mMutex);
    if (mFile.isOpen()) {
        mFile.flush();
        mFile.close();
        mCloseTimer.stop();
    }
}

void DynamicFile::openFile()
{
    QMutexLocker locker(&mMutex);
    if (!mFile.isOpen()) {
        bool isOpened = mFile.open(QFile::Append);
        if (isOpened) mCloseTimer.start();
        else DEB() << "Could not open \"" + mFile.fileName() +"\"";
    }
}

void DynamicFile::handleExisting(int backups)
{
    int bkMaxExist = 0;
    for (int i = 1; i <= backups; ++i) {
        bkMaxExist = i;
        if (!QFile(mFile.fileName()+"~"+QString::number(i)).exists())
            break;
    }
    QString destName(mFile.fileName()+"~"+QString::number(bkMaxExist));
    QFile file(destName);
    if (bkMaxExist == backups)
        file.remove();
    for (int i = backups-1; i >= 0; --i) {
        QString sourceName = mFile.fileName()+ ((i>0) ? "~"+QString::number(i) : "");
        file.setFileName(sourceName);
        if (file.exists()) file.rename(destName);
        destName = sourceName;
    }
    if (mFile.exists()) mFile.remove();
}

} // namespace studio
} // namespace gams
