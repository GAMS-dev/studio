/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include "modeldialog.h"
#include "gamsinfo.h"

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTableWidget>
#include <QTextStream>
#include <QDebug>
#include <QPair>
#include <QStandardPaths>

namespace gams {
namespace studio {

ModelDialog::ModelDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    //TODO(CW): This is a temporary logic for determine a GAMS system directory.
    //          This needs to be replaced by a common and central way of determining the GAMS system directory
    QDir gamsSysDir = QFileInfo(GAMSInfo::systemDir()).absoluteDir();

    libraryList.append(QPair<QTableWidget*, QString>(ui.twModelLibrary, gamsSysDir.filePath("gamslib_ml/gamslib.glb")));
    libraryList.append(QPair<QTableWidget*, QString>(ui.twTestLibrary,  gamsSysDir.filePath("testlib_ml/testlib.glb")));
    libraryList.append(QPair<QTableWidget*, QString>(ui.twAPILibrary,   gamsSysDir.filePath("apilib_ml/apilib.glb")));
    libraryList.append(QPair<QTableWidget*, QString>(ui.twDataLibrary,  gamsSysDir.filePath("datalib_ml/datalib.glb")));
    libraryList.append(QPair<QTableWidget*, QString>(ui.twEMPLibrary,   gamsSysDir.filePath("emplib_ml/emplib.glb")));

    QStringList errList;
    for(auto item : libraryList)
    {
        if(!populateTable(item.first, item.second))
            errList.append(item.second);
    }
    if(!errList.empty())
        QMessageBox::critical(this, "Error", "Error loading files. One or more libraries might not be available:\n" + errList.join("\n"));
}

bool ModelDialog::populateTable(QTableWidget *tw, QString glbFile)
{
    QFile file(glbFile);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QTextStream in(&file);

    in.readLine();
    in.readLine();

    // read column header
    int nrColumns = in.readLine().split("=")[1].trimmed().toInt();
    QList<int> colMap;
    for(int i=0; i<nrColumns; i++)
        colMap.append(-1);
    QStringList splitList;
    tw->setColumnCount(nrColumns);
    for(int i=0; i<nrColumns; i++)
    {
        splitList = in.readLine().split("=");
        colMap.replace(splitList.at(0).trimmed().toInt()-1, i);
        QTableWidgetItem* item = new QTableWidgetItem(splitList.at(1).trimmed());
        tw->setHorizontalHeaderItem(i, item);
    }
    //int initSortCol = in.readLine().split("=").at(1).trimmed().toInt()-1; //TODO(CW): currently no sorting since this information should not be part of the glb file

    // read models
    while(!in.atEnd()) {
        if(in.readLine().startsWith("*$*$*$"))
        {
            if(in.readLine().length() == 0) // we have reached the end of the file
                file.close();
            else // read new model
            {
                tw->insertRow(tw->rowCount());
                for(int i=0; i<nrColumns; i++)
                    tw->setItem(tw->rowCount()-1,colMap.at(i), new QTableWidgetItem(in.readLine().split("=")[1].trimmed()));
            }
        }
        else
        {
            // TODO(CW): read describtion
        }
    }
    return true;
}


void ModelDialog::on_lineEdit_textChanged(const QString &arg1)
{
    if(arg1.length() == 0)
    {
        for(auto item : libraryList)
        {
            QTableWidget* tw = item.first;
            for(int j=0; j<tw->rowCount(); j++)
                tw->showRow(j);
        }
    }
    else
    {
        for(auto item : libraryList)
        {
            QTableWidget* tw = item.first;
            for(int j=0; j<tw->rowCount(); j++)
                tw->hideRow(j);
            QList<QTableWidgetItem *> filteredItems = tw->findItems(arg1,Qt::MatchContains);
            for(auto rowPtr : filteredItems)
                tw->showRow(rowPtr->row());
        }
    }
}

}
}
