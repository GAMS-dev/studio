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

bool GlbParser::parseFile(QString glbFile)
{
    mLibraryItems.clear();
    mLineNr = 0;
    mGlbFile = glbFile;
    QFile file(mGlbFile);
    if (!file.open(QIODevice::ReadOnly))
        EXCEPT() << "GLB file '" << file.fileName() << "' not found";
    QTextStream in(&file);

    QStringList splitList;

    // create Library object containing library meta data like library name and column names
    splitList = readLine(in).split("=");
    if (!checkListSize(splitList, 2) || !checkKey(splitList[0], "Version"))
        return false;
    int version = splitList[1].trimmed().toInt();

    splitList = readLine(in).split("=");
    if (!checkListSize(splitList, 2) || !checkKey(splitList[0], "LibraryName"))
        return false;
    QString name = splitList[1].trimmed();

    splitList = readLine(in).split("=");
    if (!checkListSize(splitList, 2) || !checkKey(splitList[0], "Columns"))
        return false;
    int nrColumns = splitList[1].trimmed().toInt();

    QList<int> colOrder;
    QStringList columns;
    QStringList toolTips;
    int nameIdx = 0;
    for (int i=0; i<nrColumns; i++) {
        splitList = readLine(in).split("|");
        if(2 == splitList.size()) //we have a tool tip
            toolTips.append(splitList.at(1).trimmed());
        else
            toolTips.append("");

        splitList = splitList.at(0).split("=");
        if (!checkListSize(splitList, 2))
            return false;
        int orderNumber = splitList.at(0).trimmed().toInt();
        if (orderNumber<1) { // order numbers have to be positive
            mErrorMessage = "Error while loading model library from GLB file: " + mGlbFile + " (line " + QString::number(mLineNr) + ")";
            return false;
        }
        colOrder.append(orderNumber-1); // 1-based to 0-based
        columns.append(splitList.at(1).trimmed());
        if (columns.last().toLower() == "name")
            nameIdx = splitList.at(0).trimmed().toInt()-1;
    }
    //int initSortCol = readLine(in).split("=").at(1).trimmed().toInt()-1; //TODO(CW): currently no default sorting index. sorting index is first column
    std::shared_ptr<Library> library = std::make_shared<Library>(name, version, nrColumns, columns, toolTips, colOrder, mGlbFile);

    // read models
    QString line;
    QString description;    
    QMap<QString, int> nameDuplicates;
    line = readLine(in);
    while (!in.atEnd()) {
        if (line.startsWith("*$*$*$")) {
            line = readLine(in);
            if (line.length() == 0) // we have reached the end of the file
                file.close();
            else { // read new model
                splitList = line.split("=");
                if (!checkListSize(splitList, 2) || !checkKey(splitList[0], "Files"))
                    return false;
                QStringList files = splitList[1].trimmed().split(",");
                line = readLine(in);
                if (line.startsWith("Directory")) // skip extra line containing the source directory of the model to be retrieved
                    line = readLine(in);
                QStringList values;
                int suffixNumber = 0;
                for(int i=0; i<nrColumns; i++)
                    values << "";
                QString longDescription;
                for (int i=0; i<nrColumns; i++) {
                    splitList = line.split("=");
                    while (splitList.size()>2) { // custom implementation of a maxsplit=1
                        splitList[1] = splitList[1]+"="+splitList[2];
                        splitList.removeAt(2);
                    }
                    if (!checkListSize(splitList, 2))
                        return false;
                    int idx = splitList[0].trimmed().toInt()-1;
                    QString value = splitList[1].trimmed();
                    values[idx] = value;
                    if (idx == nameIdx) {
                        if (nameDuplicates.contains(value.toLower()))
                            suffixNumber = nameDuplicates[value.toLower()]+1;
                        nameDuplicates[value.toLower()] = suffixNumber;
                    }
                    line = readLine(in);
                }
                while (!line.startsWith("*$*$*$") && !in.atEnd()) {
                    longDescription += line + "\n";
                    line = readLine(in);
                }
                longDescription = longDescription.trimmed();
                mLibraryItems.append(LibraryItem(library, values, description, longDescription, files, suffixNumber));
            }
        }
        else
            line = readLine(in);
    }
    return true;
}

QList<LibraryItem> GlbParser::libraryItems() const
{
    return mLibraryItems;
}

QString GlbParser::errorMessage() const
{
    return mErrorMessage;
}

bool GlbParser::checkListSize(const QStringList& list, int expectedSize)
{
    if (list.size() != expectedSize) {
        mErrorMessage = "Error while loading model library from GLB file: " + mGlbFile + " (line " + QString::number(mLineNr) + ")";
        return false;
    }
    return true;
}

bool GlbParser::checkKey(QString key, QString expected)
{
    if (!key.startsWith(expected)) {
        mErrorMessage = "Error while loading model library from GLB file: " + mGlbFile + "(line " + QString::number(mLineNr) + "): Expected '" + expected + "' but found '" + key + "'";
        return false;
    }
    return true;
}

QString GlbParser::readLine(QTextStream &in)
{
    mLineNr++;
    return in.readLine();
}

} // namespace studio
} // namespace gams
