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
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTableWidget>
#include <QTextStream>
#include <QDebug>
#include <QPair>
#include <QStandardPaths>
#include "glbparser.h"
#include "libraryitem.h"
#include "librarymodel.h"

namespace gams {
namespace studio {

ModelDialog::ModelDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    //TODO(CW): This is a temporary logic for determine a GAMS system directory.
    //          This needs to be replaced by a common and central way of determining the GAMS system directory
    QDir gamsSysDir = QFileInfo(QStandardPaths::findExecutable("gams")).absoluteDir();
    gamsSysDir = QFileInfo("C:\\gams\\win64\\25.0\\").absoluteDir();

    QStringList glbFiles;
    glbFiles << gamsSysDir.filePath("gamslib_ml/gamslib.glb")
             << gamsSysDir.filePath("testlib_ml/testlib.glb")
             << gamsSysDir.filePath("apilib_ml/apilib.glb")
             << gamsSysDir.filePath("datalib_ml/datalib.glb")
             << gamsSysDir.filePath("emplib_ml/emplib.glb")
             << gamsSysDir.filePath("finlib_ml/finlib.glb")
             << gamsSysDir.filePath("noalib_ml/noalib.glb");


    QList<LibraryItem> items;
    QTableView* tableView;
    for(auto item : glbFiles)
    {
        items = GlbParser::parseFile(gamsSysDir.filePath(item));
        tableView = new QTableView();
        tableView->setModel(new LibraryModel(items, tableView));
        ui.tabWidget->addTab(tableView, items.at(0).library()->name());
    }
}

/*
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
*/

}
}
