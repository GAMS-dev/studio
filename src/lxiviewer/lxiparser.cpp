/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "lxitreeitem.h"

namespace gams {
namespace studio {
namespace lxiviewer {

LxiTreeModel *LxiParser::parseFile(const QString& lxiFile)
{
    QVector<int> lineNrs;
    QVector<LxiTreeItem*> treeItems;

    QFile file(lxiFile);
    if(!file.open(QFile::ReadOnly))
        EXCEPT() << "Unable to open file: " << lxiFile;
    QTextStream in(&file);

    QStringList splitList;
    LxiTreeItem* rootItem = new LxiTreeItem();
    LxiTreeItem* lastParent = rootItem;
    LxiTreeItem* current = nullptr;

    QString lastIdx = "B";
    while (!in.atEnd()) {
        splitList = in.readLine().split(' ');
        if (splitList.size() < 2) continue;
        QString idx = splitList.first();
        splitList.removeFirst();
        int lineNr = splitList.first().toInt();
        splitList.removeFirst();
        QString text = splitList.join(' ');

        if (idx == "B") {
            lastParent = rootItem;
        } else if (idx != lastIdx) {
            current = new LxiTreeItem(lastIdx, -1, mCaptions[idx], rootItem);
            rootItem->appendChild(current);
            lastParent = current;
        }
        current = new LxiTreeItem(idx, lineNr, text, lastParent);
        lastParent->appendChild(current);
        lineNrs.append(current->lineNr());
        treeItems.append(current);
        lastIdx = idx;
    }
    return new LxiTreeModel(rootItem, lineNrs, treeItems);
}

LxiParser::LxiParser()
{

}

QMap<QString, QString> LxiParser::initCaptions()
{
    QMap<QString, QString> map;
    map.insert("A","???");
    map.insert("B","SubTitle");
    map.insert("C","Solve Summary");
    map.insert("D","Equation");
    map.insert("E","Column");
    map.insert("F","SolEQU");
    map.insert("G","SolVAR");
    map.insert("H","Solution");
    map.insert("I","Display");
    map.insert("J","Message");
    return map;
}

QMap<QString, QString> LxiParser::mCaptions = LxiParser::initCaptions();

} // namespace lxiviewer
} // namespace studio
} // namespace gams
