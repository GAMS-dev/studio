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
namespace reference {

ReferenceViewer::ReferenceViewer(QString referenceFile, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReferenceViewer),
    mReference(new Reference(referenceFile))
{
    ui->setupUi(this);

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

    SymbolReferenceWidget* allSymbolsRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Unknown, this);
    mTabWidget->addTab(allSymbolsRefWidget, QString("All Symbols (%1)").arg(mReference->size()));

    SymbolReferenceWidget* setRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Set, this);
    mTabWidget->addTab(setRefWidget, QString("Set (%1)").arg(mReference->findReference(SymbolDataType::Set).size()));

    SymbolReferenceWidget* acronymRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Acronym, this);
    mTabWidget->addTab(acronymRefWidget, QString("Acronym (%1)").arg(mReference->findReference(SymbolDataType::Acronym).size()));

    SymbolReferenceWidget* varRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Variable, this);
    mTabWidget->addTab(varRefWidget, QString("Variable (%1)").arg(mReference->findReference(SymbolDataType::Variable).size()));

    SymbolReferenceWidget* parRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Parameter, this);
    mTabWidget->addTab(parRefWidget, QString("Parameter (%1)").arg(mReference->findReference(SymbolDataType::Parameter).size()));

    SymbolReferenceWidget* equRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Equation, this);
    mTabWidget->addTab(equRefWidget, QString("Equation (%1)").arg(mReference->findReference(SymbolDataType::Equation).size()));

    SymbolReferenceWidget* modelRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Model, this);
    mTabWidget->addTab(modelRefWidget, QString("Model (%1)").arg(mReference->findReference(SymbolDataType::Model).size()));

    SymbolReferenceWidget* fileRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::File, this);
    mTabWidget->addTab(fileRefWidget, QString("File (%1)").arg(mReference->findReference(SymbolDataType::File).size()));

    SymbolReferenceWidget* functRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Funct, this);
    mTabWidget->addTab(functRefWidget, QString("Function (%1)").arg(mReference->findReference(SymbolDataType::Funct).size()));

    SymbolReferenceWidget* unusedRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::Unused, this);
    mTabWidget->addTab(unusedRefWidget, QString("Unused (%1)").arg(mReference->findReference(SymbolDataType::Unused).size()));

    SymbolReferenceWidget* fileusedRefWidget = new SymbolReferenceWidget(mReference, SymbolDataType::FileUsed, this);
    mTabWidget->addTab(fileusedRefWidget, QString("File Used (%1)").arg(mReference->getFileUsed().size()));

    ui->referenceLayout->addWidget(mTabWidget);
    mTabWidget->setCurrentIndex(0);

    connect(mReference, &Reference::loadFinished, this, &ReferenceViewer::updateView);
}

ReferenceViewer::~ReferenceViewer()
{
    delete ui;
    delete mReference;
}

void ReferenceViewer::on_referenceFileChanged()
{
    mReference->loadReferenceFile();
}

void ReferenceViewer::updateView(bool status)
{
    if (status == Reference::LoadStatus::UnsuccesffullyLoaded)
        return;

    mTabWidget->setTabText(0, QString("All Symbols (%1)").arg(mReference->size()));
    mTabWidget->setTabText(1, QString("Set (%1)").arg(mReference->findReference(SymbolDataType::Set).size()));
    mTabWidget->setTabText(2, QString("Acronym (%1)").arg(mReference->findReference(SymbolDataType::Acronym).size()));
    mTabWidget->setTabText(3, QString("Variable (%1)").arg(mReference->findReference(SymbolDataType::Variable).size()));
    mTabWidget->setTabText(4, QString("Parameter (%1)").arg(mReference->findReference(SymbolDataType::Parameter).size()));
    mTabWidget->setTabText(5, QString("Equation (%1)").arg(mReference->findReference(SymbolDataType::Equation).size()));
    mTabWidget->setTabText(6, QString("Model (%1)").arg(mReference->findReference(SymbolDataType::Model).size()));
    mTabWidget->setTabText(7, QString("File (%1)").arg(mReference->findReference(SymbolDataType::File).size()));
    mTabWidget->setTabText(8, QString("Function (%1)").arg(mReference->findReference(SymbolDataType::Funct).size()));
    mTabWidget->setTabText(9, QString("Unused (%1)").arg(mReference->findReference(SymbolDataType::Unused).size()));
    mTabWidget->setTabText(10, QString("File Used (%1)").arg(mReference->getFileUsed().size()));
    for(int i=0; i<mTabWidget->count(); i++) {
        SymbolReferenceWidget* refWidget = static_cast<SymbolReferenceWidget*>(mTabWidget->widget(i));
        refWidget->resetModel();
    }
}

} // namespace reference
} // namespace studio
} // namespace gams
