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
#include <QDir>
#include "glbparser.h"
#include "library.h"
#include "exception.h"

namespace gams {
namespace studio {
namespace modeldialog {

GlbParser::GlbParser()
{

}

bool GlbParser::parseFile(const QString &glbFile)
{
    mLibraryItems.clear();
    mLineNr = 0;
    mGlbFile = glbFile;
    QFile file(mGlbFile);
    if (!file.open(QFile::ReadOnly))
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
        if (orderNumber<1 || orderNumber>nrColumns) { // order numbers have to be positive
            mErrorMessage = "Error while loading model library from GLB file. Found column number(" + QString::number(orderNumber) + ") that is out of valid range (1.." + QString::number(nrColumns) + "): " + mGlbFile + " (line " + QString::number(mLineNr) + ")";
            return false;
        }

        colOrder.append(orderNumber-1); // 1-based to 0-based
        columns.append(splitList.at(1).trimmed());
        if (columns.last().toLower() == "name")
            nameIdx = splitList.at(0).trimmed().toInt()-1;
    }
    splitList = readLine(in).split("=");
    if (!checkListSize(splitList, 2) || !checkKey(splitList[0], "InitialSort"))
        return false;
    int initSortCol = splitList[1].trimmed().toInt() - 1;
    QSharedPointer<Library> library(new Library(name, version, nrColumns, columns, initSortCol, toolTips, colOrder, mGlbFile));

    // read models
    QString line;
    QString description;    
    QMap<QString, int> nameDuplicates;
    line = readLine(in);
    while (!in.atEnd()) {
        if (line.startsWith("*$*$*$")) {
            line = readLine(in);
            if(line.trimmed().isEmpty()) // stop reading the file if we hit an empty line. This is what the IDE and also gamslib do
                return true;
            if (line.length() == 0) // we have reached the end of the file
                file.close();
            else { // read new model
                splitList = line.split("=");
                if (!checkListSize(splitList, 2) || !checkKey(splitList[0], "Files"))
                    return false;
                QStringList files = splitList[1].trimmed().split(",");
                for (int i=0; i<files.size(); i++)
                    files[i] = files[i].trimmed();
                files.removeAll("");

                if (files.isEmpty()) { // do not allow empty list of files
                    mErrorMessage = "Error while loading model library from GLB file. Found the specification of an empty file list: " + mGlbFile + " (line " + QString::number(mLineNr) + ")";
                    return false;
                }
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
                    int idx = splitList[0].trimmed().toInt();
                    if (idx<1 || idx>nrColumns) {
                        mErrorMessage = "Error while loading model library from GLB file. Found column number(" + QString::number(idx) + ") that is out of valid range (1.." + QString::number(nrColumns) + "): " + mGlbFile + " (line " + QString::number(mLineNr) + ")";
                        return false;
                    }
                    idx--;
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
    // we need to have at least one model, otherwise the GLB file is not valid
    if (mLibraryItems.isEmpty()) {
        mErrorMessage = "Error while loading model library from GLB file. Library is empty but needs to contain at least one model: " + mGlbFile + " (line " + QString::number(mLineNr) + ")";
        return false;
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

bool GlbParser::checkKey(const QString &key, const QString &expected)
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

} // namespace modeldialog
} // namespace studio
} // namespace gams
