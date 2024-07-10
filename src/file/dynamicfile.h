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
#ifndef DYNAMICFILE_H
#define DYNAMICFILE_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QMutex>

namespace gams {
namespace studio {

class DynamicFile : public QObject
{
    Q_OBJECT
public:
    DynamicFile(const QString &fileName, int backups = 0, QObject *parent = nullptr);
    virtual ~DynamicFile();
    void appendLine(const QString &line);
    void confirmLastLine();

private slots:
    void closeFile();

private:
    void openFile();
    void runBackupCircle();

private:
    QMutex mMutex;
    QFile mFile;
    int mBackups = 0;
    qint64 mEnd = 0;
};

} // namespace studio
} // namespace gams

#endif // DYNAMICFILE_H
