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

#include <QDir>
#include "exception.h"
#include "lxiparser.h"


namespace gams {
namespace studio {
namespace lxiviewer {

LxiTreeItem *LxiParser::parseFile(QString lxiFile)
{
    lxiFile = QDir::toNativeSeparators(lxiFile);
    QFile file(lxiFile);
    if(!file.open(QIODevice::ReadOnly))
        EXCEPT() << "Unable to open file: " << lxiFile;
    QTextStream in(&file);

    QStringList splitList;
    LxiTreeItem* rootItem = new LxiTreeItem();
    LxiTreeItem* lastParent = rootItem;
    LxiTreeItem* current = nullptr;

    QString lastIdx = "B";
    while (!in.atEnd()) {
        splitList = in.readLine().split(' ');
        QString idx = splitList.first();
        splitList.removeFirst();
        int lineNr = splitList.first().toInt();
        splitList.removeFirst();
        QString text = splitList.join(' ');

        if (idx == lastIdx)
            lastParent->appendChild(new LxiTreeItem(idx, lineNr, text, lastParent));
        else {
            if (idx == "D")
                current = new LxiTreeItem(lastIdx, -1, "Equation", lastParent);
            else if (idx == "E")
                current = new LxiTreeItem(lastIdx, -1, "Column", lastParent);
            else if (idx == "F")
                current = new LxiTreeItem(lastIdx, -1, "SolEQU", lastParent);
            else if (idx == "G")
                current = new LxiTreeItem(lastIdx, -1, "SolVAR", lastParent);

            lastParent = current;
            lastParent->appendChild(new LxiTreeItem(idx, lineNr, text, lastParent));
        }
        lastIdx = idx;
    }
}

LxiParser::LxiParser()
{

}

} // namespace lxiviewer
} // namespace studio
} // namespace gams
