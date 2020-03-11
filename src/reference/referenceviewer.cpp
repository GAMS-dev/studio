/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "symbolreferencewidget.h"

namespace gams {
namespace studio {
namespace reference {

ReferenceViewer::ReferenceViewer(QString referenceFile, QTextCodec* codec, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReferenceViewer),
    mCodec(codec),
    mReference(new Reference(referenceFile, codec)),
    mRefTabStyle(QSharedPointer<ReferenceTabStyle>(new ReferenceTabStyle(QApplication::style()), &QObject::deleteLater))
{
    ui->setupUi(this);

    ui->tabWidget->tabBar()->setStyle(mRefTabStyle.data());

    SymbolReferenceWidget* allSymbolsRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::Unknown, this);
    ui->tabWidget->addTab(allSymbolsRefWidget, QString("All Symbols (%1)").arg(mReference->size()));

    SymbolReferenceWidget* setRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::Set, this);
    ui->tabWidget->addTab(setRefWidget, QString("Set (%1)").arg(mReference->findReference(SymbolDataType::Set).size()));

    SymbolReferenceWidget* acronymRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::Acronym, this);
    ui->tabWidget->addTab(acronymRefWidget, QString("Acronym (%1)").arg(mReference->findReference(SymbolDataType::Acronym).size()));

    SymbolReferenceWidget* varRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::Variable, this);
    ui->tabWidget->addTab(varRefWidget, QString("Variable (%1)").arg(mReference->findReference(SymbolDataType::Variable).size()));

    SymbolReferenceWidget* parRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::Parameter, this);
    ui->tabWidget->addTab(parRefWidget, QString("Parameter (%1)").arg(mReference->findReference(SymbolDataType::Parameter).size()));

    SymbolReferenceWidget* equRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::Equation, this);
    ui->tabWidget->addTab(equRefWidget, QString("Equation (%1)").arg(mReference->findReference(SymbolDataType::Equation).size()));

    SymbolReferenceWidget* modelRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::Model, this);
    ui->tabWidget->addTab(modelRefWidget, QString("Model (%1)").arg(mReference->findReference(SymbolDataType::Model).size()));

    SymbolReferenceWidget* fileRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::File, this);
    ui->tabWidget->addTab(fileRefWidget, QString("File (%1)").arg(mReference->findReference(SymbolDataType::File).size()));

    SymbolReferenceWidget* functRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::Funct, this);
    ui->tabWidget->addTab(functRefWidget, QString("Function (%1)").arg(mReference->findReference(SymbolDataType::Funct).size()));

    SymbolReferenceWidget* unusedRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::Unused, this);
    ui->tabWidget->addTab(unusedRefWidget, QString("Unused (%1)").arg(mReference->findReference(SymbolDataType::Unused).size()));

    SymbolReferenceWidget* fileusedRefWidget = new SymbolReferenceWidget(mReference.data(), SymbolDataType::FileUsed, this);
    ui->tabWidget->addTab(fileusedRefWidget, QString("File Used (%1)").arg(mReference->getFileUsed().size()));

    ui->tabWidget->setCurrentIndex(0);
    allSymbolsRefWidget->initModel();
    setFocusProxy(ui->tabWidget);

    connect(ui->tabWidget, &QTabWidget::tabBarClicked, this, &ReferenceViewer::on_tabBarClicked);
    connect(mReference.data(), &Reference::loadFinished, this, &ReferenceViewer::updateView);
}

ReferenceViewer::~ReferenceViewer()
{
    delete ui;
}

void ReferenceViewer::selectSearchField() const
{
    SymbolReferenceWidget* tabWidget = static_cast<SymbolReferenceWidget*>(ui->tabWidget->currentWidget());
    if (tabWidget)
        tabWidget->selectSearchField();
}

void ReferenceViewer::on_referenceFileChanged(QTextCodec* codec)
{
    mReference->loadReferenceFile(codec);
    for(int i=0; i<ui->tabWidget->count(); i++) {
        SymbolReferenceWidget* refWidget = static_cast<SymbolReferenceWidget*>(ui->tabWidget->widget(i));
        refWidget->resetModel();
    }
}

void ReferenceViewer::on_tabBarClicked(int index)
{
    SymbolReferenceWidget* refWidget = static_cast<SymbolReferenceWidget*>(ui->tabWidget->widget(index));
    if (refWidget && !refWidget->isModelLoaded())
        refWidget->initModel();
}

void ReferenceViewer::updateView(bool status)
{
    if (status == Reference::LoadedState::UnsuccesffullyLoaded)
        return;

    ui->tabWidget->setTabText(0, QString("All Symbols (%1)").arg(mReference->size()));
    ui->tabWidget->setTabText(1, QString("Set (%1)").arg(mReference->findReference(SymbolDataType::Set).size()));
    ui->tabWidget->setTabText(2, QString("Acronym (%1)").arg(mReference->findReference(SymbolDataType::Acronym).size()));
    ui->tabWidget->setTabText(3, QString("Variable (%1)").arg(mReference->findReference(SymbolDataType::Variable).size()));
    ui->tabWidget->setTabText(4, QString("Parameter (%1)").arg(mReference->findReference(SymbolDataType::Parameter).size()));
    ui->tabWidget->setTabText(5, QString("Equation (%1)").arg(mReference->findReference(SymbolDataType::Equation).size()));
    ui->tabWidget->setTabText(6, QString("Model (%1)").arg(mReference->findReference(SymbolDataType::Model).size()));
    ui->tabWidget->setTabText(7, QString("File (%1)").arg(mReference->findReference(SymbolDataType::File).size()));
    ui->tabWidget->setTabText(8, QString("Function (%1)").arg(mReference->findReference(SymbolDataType::Funct).size()));
    ui->tabWidget->setTabText(9, QString("Unused (%1)").arg(mReference->findReference(SymbolDataType::Unused).size()));
    ui->tabWidget->setTabText(10, QString("File Used (%1)").arg(mReference->getFileUsed().size()));
}

} // namespace reference
} // namespace studio
} // namespace gams
