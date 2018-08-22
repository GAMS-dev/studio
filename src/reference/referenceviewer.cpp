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
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTabBar>
#include <QTextStream>
#include <QStackedWidget>

#include "referenceviewer.h"
#include "ui_referenceviewer.h"
#include "referencetabstyle.h"
#include "symbolreferenceitem.h"

namespace gams {
namespace studio {

ReferenceViewer::ReferenceViewer(QString referenceFile, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReferenceViewer)
{
    ui->setupUi(this);

    Reference* reference = new Reference(referenceFile);

    mTabWidget =  new QTabWidget(this);
    mTabWidget->setObjectName(QStringLiteral("tabWidget"));
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(3);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(mTabWidget->sizePolicy().hasHeightForWidth());
    mTabWidget->setSizePolicy(sizePolicy);
    mTabWidget->setTabsClosable(false);
    mTabWidget->setTabBarAutoHide(false);
    mTabWidget->setTabPosition(QTabWidget::West);
    mTabWidget->tabBar()->setStyle( new ReferenceTabStyle );

    SymbolReferenceWidget* allSymbolsRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::Unknown, this);
    mTabWidget->addTab(allSymbolsRefWidget, QString("All Symbols (%1)").arg(reference->size()));

    SymbolReferenceWidget* setRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::Set, this);
    mTabWidget->addTab(setRefWidget, QString("Set (%1)").arg(reference->findReference(SymbolDataType::Set).size()));

    SymbolReferenceWidget* acronymRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::Acronym, this);
    mTabWidget->addTab(acronymRefWidget, QString("Acronym (%1)").arg(reference->findReference(SymbolDataType::Acronym).size()));

    SymbolReferenceWidget* varRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::Variable, this);
    mTabWidget->addTab(varRefWidget, QString("Variable (%1)").arg(reference->findReference(SymbolDataType::Variable).size()));

    SymbolReferenceWidget* parRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::Parameter, this);
    mTabWidget->addTab(parRefWidget, QString("Parameter (%1)").arg(reference->findReference(SymbolDataType::Parameter).size()));

    SymbolReferenceWidget* equRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::Equation, this);
    mTabWidget->addTab(equRefWidget, QString("Equation (%1)").arg(reference->findReference(SymbolDataType::Equation).size()));

    SymbolReferenceWidget* modelRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::Model, this);
    mTabWidget->addTab(modelRefWidget, QString("Model (%1)").arg(reference->findReference(SymbolDataType::Model).size()));

    SymbolReferenceWidget* fileRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::File, this);
    mTabWidget->addTab(fileRefWidget, QString("File (%1)").arg(reference->findReference(SymbolDataType::File).size()));

    SymbolReferenceWidget* functRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::Funct, this);
    mTabWidget->addTab(functRefWidget, QString("Function (%1)").arg(reference->findReference(SymbolDataType::Funct).size()));

    SymbolReferenceWidget* unusedRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::Unused, this);
    mTabWidget->addTab(unusedRefWidget, QString("Unused (%1)").arg(reference->findReference(SymbolDataType::Unused).size()));

    SymbolReferenceWidget* fileusedRefWidget = new SymbolReferenceWidget(reference, SymbolDataType::FileUsed, this);
    mTabWidget->addTab(fileusedRefWidget, QString("File Used (%1)").arg(reference->getFileUsed().size()));

    ui->referenceLayout->addWidget(mTabWidget);
    mTabWidget->setCurrentIndex(0);
}

ReferenceViewer::~ReferenceViewer()
{
    delete ui;
}

} // namespace studio
} // namespace gams
