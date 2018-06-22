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
#include "glbparser.h"
#include "library.h"
#include "exception.h"

namespace gams {
namespace studio {

GlbParser::GlbParser()
{

}

QList<LibraryItem> GlbParser::parseFile(QString glbFile)
{
    glbFile = QDir::toNativeSeparators(glbFile);
    QFile file(glbFile);
    if (!file.open(QIODevice::ReadOnly))
        EXCEPT() << "GLB file '" << file.fileName() << "' not found";
    QTextStream in(&file);

    // create Library object containing library meta data like library name and column names
    int version = in.readLine().split("=")[1].trimmed().toInt();
    QString name = in.readLine().split("=")[1].trimmed();
    int nrColumns = in.readLine().split("=")[1].trimmed().toInt();
    QList<int> colOrder;
    QStringList splitList;
    QStringList columns;
    QStringList toolTips;
    for (int i=0; i<nrColumns; i++) {
        splitList = in.readLine().split("|");
        if(2 == splitList.size()) //we have a tool tip
            toolTips.append(splitList.at(1).trimmed());
        else
            toolTips.append("");
        splitList = splitList.at(0).split("=");
        colOrder.append(splitList.at(0).trimmed().toInt()-1);
        columns.append(splitList.at(1).trimmed());
    }
    //int initSortCol = in.readLine().split("=").at(1).trimmed().toInt()-1; //TODO(CW): currently no default sorting index. sorting index is first column
    std::shared_ptr<Library> library = std::make_shared<Library>(name, version, nrColumns, columns, toolTips, colOrder, glbFile);

    // read models
    QList<LibraryItem> libraryItems;
    QString line;
    QString description;
    line = in.readLine();
    while (!in.atEnd()) {
        if (line.startsWith("*$*$*$")) {
            line = in.readLine();
            if (line.length() == 0) // we have reached the end of the file
                file.close();
            else { // read new model
                QStringList files = line.split("=")[1].trimmed().split(",");
                line = in.readLine();
                if (line.startsWith("Directory", Qt::CaseInsensitive)) // skip extra line containing the source directory of the model to be retrieved
                    line = in.readLine();
                QStringList values;
                QString longDescription;
                for (int i=0; i<nrColumns; i++) {
                    values.append(line.split("=")[1].trimmed());
                    line = in.readLine();
                }
                while (!line.startsWith("*$*$*$") && !in.atEnd()) {
                    longDescription += line + "\n";
                    line = in.readLine();
                }
                longDescription = longDescription.trimmed();
                libraryItems.append(LibraryItem(library, values, description, longDescription, files));
            }
        }
        else
            line = in.readLine();
    }
    return libraryItems;
}

} // namespace studio
} // namespace gams
