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
#include <QSortFilterProxyModel>
#include "glbparser.h"
#include "libraryitem.h"
#include "librarymodel.h"

namespace gams {
namespace studio {

ModelDialog::ModelDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    QDir gamsSysDir(GAMSInfo::systemDir());

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
    QSortFilterProxyModel* proxyModel;
    for(auto item : glbFiles)
    {
        items = GlbParser::parseFile(gamsSysDir.filePath(item));
        tableView = new QTableView();
        tableView->horizontalHeader()->setStretchLastSection(true);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->verticalHeader()->hide();
        tableView->setSortingEnabled(true);

        proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setFilterKeyColumn(-1);
        proxyModel->setSourceModel(new LibraryModel(items, this));

        connect(ui.lineEdit, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterFixedString);
        proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

        tableView->setModel(proxyModel);
        ui.tabWidget->addTab(tableView, items.at(0).library()->name());
    }
}

}
}


