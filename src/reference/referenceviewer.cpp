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
    ui(new Ui::ReferenceViewer),
    mReference(new Reference(referenceFile))
{
    ui->setupUi(this);

    mReference->dumpAll();

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

    QList<SymbolReferenceItem*> setList = mReference->findReference(SymbolDataType::Set);
    QList<SymbolReferenceItem*> acronymList = mReference->findReference(SymbolDataType::Acronym);
    QList<SymbolReferenceItem*> varList = mReference->findReference(SymbolDataType::Variable);
    QList<SymbolReferenceItem*> parList = mReference->findReference(SymbolDataType::Parameter);
    QList<SymbolReferenceItem*> equList = mReference->findReference(SymbolDataType::Equation);
    QList<SymbolReferenceItem*> fileList = mReference->findReference(SymbolDataType::File);
    QList<SymbolReferenceItem*> modelList = mReference->findReference(SymbolDataType::Model);
    QList<SymbolReferenceItem*> functList = mReference->findReference(SymbolDataType::Funct);

    SymbolReferenceWidget* allSymbolsRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Undefined, this);
    mTabWidget->addTab(allSymbolsRefWidget, QString("All Symbols (%1)").arg(mReference->size()));

    SymbolReferenceWidget* setRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Set, this);
    mTabWidget->addTab(setRefWidget, QString("Set (%1)").arg(setList.size()));

    SymbolReferenceWidget* acronymRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Acronym, this);
    mTabWidget->addTab(acronymRefWidget, QString("Acronym (%1)").arg(acronymList.size()));

    SymbolReferenceWidget* varRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Variable, this);
    mTabWidget->addTab(varRefWidget, QString("Variable (%1)").arg(varList.size()));

    SymbolReferenceWidget* parRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Parameter, this);
    mTabWidget->addTab(parRefWidget, QString("Parameter (%1)").arg(parList.size()));

    SymbolReferenceWidget* equRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Equation, this);
    mTabWidget->addTab(equRefWidget, QString("Equation (%1)").arg(equList.size()));

    SymbolReferenceWidget* modelRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Model, this);
    mTabWidget->addTab(modelRefWidget, QString("Model (%1)").arg(modelList.size()));

    SymbolReferenceWidget* fileRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::File, this);
    mTabWidget->addTab(fileRefWidget, QString("File Used (%1)").arg(fileList.size()));

    SymbolReferenceWidget* functRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Funct, this);
    mTabWidget->addTab(functRefWidget, QString("Function used (%1)").arg(functList.size()));

    ui->referenceLayout->addWidget(mTabWidget);
    mTabWidget->setCurrentIndex(0);
}

ReferenceViewer::~ReferenceViewer()
{
    delete ui;
}

} // namespace studio
} // namespace gams
