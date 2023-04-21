/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include <QFile>
#include <QTabBar>
#include <QTextStream>
#include <QStackedWidget>
#include <QWheelEvent>

#include "referenceviewer.h"
#include "ui_referenceviewer.h"
#include "filereferencewidget.h"
#include "referencetabstyle.h"
#include "symbolreferenceitem.h"
#include "symbolreferencewidget.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

namespace gams {
namespace studio {
namespace reference {

inline static SymbolReferenceWidget* initViewerType(SymbolReferenceWidget* w) {
    if(w) w->setProperty("ViewerType", int(ReferenceViewerType::Symbol));
    return w;
}

inline static FileReferenceWidget* initViewerType(FileReferenceWidget* w) {
    if(w) w->setProperty("ViewerType", int(ReferenceViewerType::FileUsed));
    return w;
}

inline static ReferenceViewerType viewerType(QWidget* w) {
    if (!w) return ReferenceViewerType::undefined;
    QVariant v = w ? w->property("ViewerType") : QVariant();
    return (v.isValid() ? static_cast<ReferenceViewerType>(v.toInt()) : ReferenceViewerType::undefined);
}

inline static SymbolReferenceWidget* toSymbolReferenceWidget(QWidget* w) {
    ReferenceViewerType t = viewerType(w);
    return (t == ReferenceViewerType::Symbol) ? static_cast<SymbolReferenceWidget*>(w) : nullptr;
}

inline static FileReferenceWidget* toFileUsedReferenceWidget(QWidget* w) {
    ReferenceViewerType t = viewerType(w);
    return (t == ReferenceViewerType::FileUsed) ? static_cast<FileReferenceWidget*>(w) : nullptr;
}

ReferenceViewer::ReferenceViewer(QString referenceFile, QString encodingName, QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::ReferenceViewer),
    mReference(new Reference(referenceFile, encodingName))
{
    ui->setupUi(this);
    updateStyle();

    bool problemLoaded = (mReference->state() == Reference::UnsuccessfullyLoaded);
    if (problemLoaded) {
        QString errorLine = (mReference->errorLine() > 0 ? QString(":%1").arg(mReference->errorLine()) : "");
        SysLogLocator::systemLog()->append(
                    QString("Error while loading: %1%2, the file content might be corrupted or incorrectly overwritten")
                              .arg(mReference->getFileLocation(), errorLine),
                    LogMsgType::Error);
    }

    QList<QHeaderView*> headers;

    SymbolReferenceWidget* allSymbolsRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Unknown, this));
    ui->tabWidget->addTab(allSymbolsRefWidget, QString("All Symbols (%1)").arg( problemLoaded ? "?" : QString::number(mReference->size())) );
    headers << allSymbolsRefWidget->headers();

    SymbolReferenceWidget* setRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Set, this));
    ui->tabWidget->addTab(setRefWidget, QString("Set (%1)").arg( problemLoaded ? "?" : QString::number(mReference->findReferenceFromType(SymbolDataType::Set).size())) );
    headers << setRefWidget->headers();

    SymbolReferenceWidget* acronymRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Acronym, this));
    ui->tabWidget->addTab(acronymRefWidget, QString("Acronym (%1)").arg( problemLoaded ? "?" : QString::number(mReference->findReferenceFromType(SymbolDataType::Acronym).size())) );
    headers << acronymRefWidget->headers();

    SymbolReferenceWidget* varRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Variable, this));
    ui->tabWidget->addTab(varRefWidget, QString("Variable (%1)").arg( problemLoaded ? "?" : QString::number(mReference->findReferenceFromType(SymbolDataType::Variable).size()) ));
    headers << varRefWidget->headers();

    SymbolReferenceWidget* parRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Parameter, this));
    ui->tabWidget->addTab(parRefWidget, QString("Parameter (%1)").arg( problemLoaded ? "?" : QString::number(mReference->findReferenceFromType(SymbolDataType::Parameter).size())) );
    headers << parRefWidget->headers();

    SymbolReferenceWidget* equRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Equation, this));
    ui->tabWidget->addTab(equRefWidget, QString("Equation (%1)").arg( problemLoaded ? "?" : QString::number(mReference->findReferenceFromType(SymbolDataType::Equation).size())) );
    headers << equRefWidget->headers();

    SymbolReferenceWidget* modelRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Model, this));
    ui->tabWidget->addTab(modelRefWidget, QString("Model (%1)").arg( problemLoaded ? "?" : QString::number(mReference->findReferenceFromType(SymbolDataType::Model).size())) );
    headers << modelRefWidget->headers();

    SymbolReferenceWidget* fileRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::File, this));
    ui->tabWidget->addTab(fileRefWidget, QString("File (%1)").arg( problemLoaded ? "?" : QString::number(mReference->findReferenceFromType(SymbolDataType::File).size())) );
    headers << fileRefWidget->headers();

    SymbolReferenceWidget* macroRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Macro, this));
    ui->tabWidget->addTab(macroRefWidget, QString("Macro (%1)").arg( problemLoaded ? "?" : QString::number(mReference->findReferenceFromType(SymbolDataType::Macro).size())) );
    headers << macroRefWidget->headers();

    SymbolReferenceWidget* functRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Funct, this));
    ui->tabWidget->addTab(functRefWidget, QString("Function (%1)").arg( problemLoaded ? "?" : QString::number(mReference->findReferenceFromType(SymbolDataType::Funct).size())) );
    headers << functRefWidget->headers();

    SymbolReferenceWidget* unusedRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Unused, this));
    ui->tabWidget->addTab(unusedRefWidget, QString("Unused (%1)").arg( problemLoaded ? "?" : QString::number(mReference->findReferenceFromType(SymbolDataType::Unused).size())) );
    headers << unusedRefWidget->headers();

    FileReferenceWidget* fileusedRefWidget = initViewerType(new FileReferenceWidget(mReference.data(), this));
    ui->tabWidget->addTab(fileusedRefWidget, QString("File Used (%1)").arg( problemLoaded ? "?" : QString::number(mReference->getNumberOfFileUsed())) );
    headers << fileusedRefWidget->headers();

    ui->tabWidget->setCurrentIndex(0);
    if (!problemLoaded) {
        ui->tabWidget->setEnabled(true);
        allSymbolsRefWidget->initModel();
    } else {
        ui->tabWidget->setEnabled(false);
    }
    setFocusProxy(ui->tabWidget);

    for (QHeaderView *header : qAsConst(headers)) {
        headerRegister(header);
    }

    connect(ui->tabWidget, &QTabWidget::tabBarClicked, this, &ReferenceViewer::on_tabBarClicked);
    connect(mReference.data(), &Reference::loadFinished, this, &ReferenceViewer::updateView);
}

ReferenceViewer::~ReferenceViewer()
{
    delete ui;
}

void ReferenceViewer::updateStyle()
{
    mRefTabStyle.reset(new ReferenceTabStyle(QApplication::style()->objectName()));
    ui->tabWidget->tabBar()->setStyle(mRefTabStyle.data());
}

void ReferenceViewer::selectSearchField() const
{
    SymbolReferenceWidget* tabWidget = static_cast<SymbolReferenceWidget*>(ui->tabWidget->currentWidget());
    if (tabWidget)
        tabWidget->selectSearchField();
}

void ReferenceViewer::reloadFile(QString encodingName)
{
    mReference->loadReferenceFile(encodingName);
    if (mReference->state() == Reference::UnsuccessfullyLoaded) {
        QString errorLine = (mReference->errorLine() > 0 ? QString(":%1").arg(mReference->errorLine()) : "");
        SysLogLocator::systemLog()->append(
                    QString("Error while reloading: %1%2, the file content might be corrupted or incorrectly overwritten")
                               .arg(mReference->getFileLocation(), errorLine),
                    LogMsgType::Error);
    }
}

void ReferenceViewer::on_tabBarClicked(int index)
{
    SymbolReferenceWidget* refWidget = toSymbolReferenceWidget(ui->tabWidget->widget(index));
    if (refWidget && !refWidget->isModelLoaded()) {
        refWidget->initModel();
    } else {
        FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->tabWidget->widget(index));
        if (fileUsedWidget && !fileUsedWidget->isModelLoaded()) {
            fileUsedWidget->initModel();
        }
    }
}

void ReferenceViewer::updateView(bool status)
{
    for(int i=0; i<ui->tabWidget->count(); i++) {
        SymbolReferenceWidget* refWidget = toSymbolReferenceWidget(ui->tabWidget->widget(i));
        if (refWidget) {
            refWidget->initModel(mReference.data());
            refWidget->resetModel();
        } else {
            FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->tabWidget->widget(i));
            if (fileUsedWidget) {
                fileUsedWidget->initModel(mReference.data());
                fileUsedWidget->resetModel();
            }
        }
    }
    if (status) {
        ui->tabWidget->setTabText(0, QString("All Symbols (%1)").arg(mReference->size()));
        ui->tabWidget->setTabText(1, QString("Set (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Set).size()));
        ui->tabWidget->setTabText(2, QString("Acronym (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Acronym).size()));
        ui->tabWidget->setTabText(3, QString("Variable (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Variable).size()));
        ui->tabWidget->setTabText(4, QString("Parameter (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Parameter).size()));
        ui->tabWidget->setTabText(5, QString("Equation (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Equation).size()));
        ui->tabWidget->setTabText(6, QString("Model (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Model).size()));
        ui->tabWidget->setTabText(7, QString("File (%1)").arg(mReference->findReferenceFromType(SymbolDataType::File).size()));
        ui->tabWidget->setTabText(8, QString("Macro (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Macro).size()));
        ui->tabWidget->setTabText(9, QString("Function (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Funct).size()));
        ui->tabWidget->setTabText(10, QString("Unused (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Unused).size()));
        ui->tabWidget->setTabText(11, QString("File Used (%1)").arg(mReference->getNumberOfFileUsed()));
    } else {
        ui->tabWidget->setTabText(0, QString("All Symbols (?)"));
        ui->tabWidget->setTabText(1, QString("Set (?)"));
        ui->tabWidget->setTabText(2, QString("Acronym (?)"));
        ui->tabWidget->setTabText(3, QString("Variable (?)"));
        ui->tabWidget->setTabText(4, QString("Parameter (?)"));
        ui->tabWidget->setTabText(5, QString("Equation (?)"));
        ui->tabWidget->setTabText(6, QString("Model (?)"));
        ui->tabWidget->setTabText(7, QString("File (?)"));
        ui->tabWidget->setTabText(8, QString("Macro (?)"));
        ui->tabWidget->setTabText(9, QString("Function (?)"));
        ui->tabWidget->setTabText(10, QString("Unused (?)"));
        ui->tabWidget->setTabText(11, QString("File Used (?)"));
        ui->tabWidget->setCurrentIndex(0);
    }
    ui->tabWidget->setEnabled(status);
}

} // namespace reference
} // namespace studio
} // namespace gams
