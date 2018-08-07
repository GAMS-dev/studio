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

    mReferenceFile = QDir::toNativeSeparators(referenceFile);
    mValid = parseFile(mReferenceFile);

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

    QList<SymbolReferenceItem*> setList = findReference(SymbolDataType::Set);
    QList<SymbolReferenceItem*> acronymList = findReference(SymbolDataType::Acronym);
    QList<SymbolReferenceItem*> varList = findReference(SymbolDataType::Variable);
    QList<SymbolReferenceItem*> parList = findReference(SymbolDataType::Parameter);
    QList<SymbolReferenceItem*> equList = findReference(SymbolDataType::Equation);
    QList<SymbolReferenceItem*> fileList = findReference(SymbolDataType::File);
    QList<SymbolReferenceItem*> modelList = findReference(SymbolDataType::Model);
    QList<SymbolReferenceItem*> functList = findReference(SymbolDataType::Funct);

    if (mReference.size() > 0) {
         SymbolReferenceWidget* allSymbolsRefWidget = new SymbolReferenceWidget(SymbolDataType::from(SymbolDataType::Undefined), this);
         mTabWidget->addTab(allSymbolsRefWidget, QString("All Symbols (%1)").arg(mReference.size()));
    }
    if (acronymList.size() > 0) {
        SymbolReferenceWidget* setRefWidget = new SymbolReferenceWidget(SymbolDataType::from(SymbolDataType::Set), this);
        mTabWidget->addTab(setRefWidget, QString("Set (%1)").arg(setList.size()));
    }
    if (acronymList.size() > 0) {
        SymbolReferenceWidget* acronymRefWidget = new SymbolReferenceWidget(SymbolDataType::from(SymbolDataType::Acronym), this);
        mTabWidget->addTab(acronymRefWidget, QString("Acronym (%1)").arg(acronymList.size()));
    }
    if (varList.size() > 0) {
        SymbolReferenceWidget* varRefWidget = new SymbolReferenceWidget(SymbolDataType::from(SymbolDataType::Variable), this);
        mTabWidget->addTab(varRefWidget, QString("Variable (%1)").arg(varList.size()));
    }
    if (parList.size() > 0) {
        SymbolReferenceWidget* parRefWidget = new SymbolReferenceWidget(SymbolDataType::from(SymbolDataType::Parameter), this);
        mTabWidget->addTab(parRefWidget, QString("Parameter (%1)").arg(parList.size()));
    }
    if (equList.size() > 0) {
        SymbolReferenceWidget* equRefWidget = new SymbolReferenceWidget(SymbolDataType::from(SymbolDataType::Equation), this);
        mTabWidget->addTab(equRefWidget, QString("Equation (%1)").arg(equList.size()));
    }
    if (fileList.size() > 0) {
        SymbolReferenceWidget* fileRefWidget = new SymbolReferenceWidget(SymbolDataType::from(SymbolDataType::File), this);
        mTabWidget->addTab(fileRefWidget, QString("File (%1)").arg(fileList.size()));
    }
    if (modelList.size() > 0) {
        SymbolReferenceWidget* modelRefWidget = new SymbolReferenceWidget(SymbolDataType::from(SymbolDataType::Model), this);
        mTabWidget->addTab(modelRefWidget, QString("Model (%1)").arg(modelList.size()));
    }
    if (functList.size() > 0) {
        SymbolReferenceWidget* functRefWidget = new SymbolReferenceWidget(SymbolDataType::from(SymbolDataType::Funct), this);
        mTabWidget->addTab(functRefWidget, QString("Function (%1)").arg(functList.size()));
    }
    ui->referenceLayout->addWidget(mTabWidget);
    mTabWidget->setCurrentIndex(0);

}

ReferenceViewer::~ReferenceViewer()
{
    delete ui;
}

bool ReferenceViewer::parseFile(QString referenceFile)
{
    QFile file(referenceFile);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "cannot open file [" << referenceFile << "]";
        return false;
    }
    QTextStream in(&file);

    QStringList recordList;
    QString idx;
    while (!in.atEnd()) {
        recordList = in.readLine().split(' ');
        idx = recordList.first();
        if (idx.toInt()== 0)
            break;
        recordList.removeFirst();
        QString id = recordList.at(0);
        QString symbolName = recordList.at(1);
        QString symbolType = recordList.at(2);
        QString referenceType = recordList.at(3);
        QString lineNumber = recordList.at(5);
        QString columnNumber = recordList.at(6);
        QString location = recordList.at(9);

        SymbolDataType::SymbolType type = SymbolDataType::typeFrom(symbolType);
        if (!mReference.contains(id.toInt()))
            mReference[id.toInt()] = new SymbolReferenceItem(id.toInt(), symbolName, type);
        SymbolReferenceItem* ref = mReference[id.toInt()];
        addReferenceInfo(ref, referenceType, lineNumber.toInt(), columnNumber.toInt(), location);
    }
    if (in.atEnd())
        return false;

    recordList.removeFirst();
    int size = recordList.first().toInt();
    while (!in.atEnd()) {
        recordList = in.readLine().split(' ');
        idx = recordList.first();
        QString id = recordList.at(0);
        QString symbolName = recordList.at(1);
        QString symbolType = recordList.at(3);
        QString dimension = recordList.at(4);
        QString numberOfElements = recordList.at(5);

        SymbolDataType::SymbolType type = SymbolDataType::typeFrom(symbolType);
        if (!mReference.contains(id.toInt()))
            mReference[id.toInt()] = new SymbolReferenceItem(id.toInt(), symbolName, type);
        SymbolReferenceItem* ref = mReference[id.toInt()];
        ref->setDimension(dimension.toInt());
        mSymbolNameMap[symbolName] = id.toInt();

        QList<SymbolId> domain;
        if (dimension.toInt() > 0) {
            for(int dim=0; dim < dimension.toInt(); dim++) {
                QString d = recordList.at(6+dim);
                domain << d.toInt();
           }
        } // do not have dimension reference if dimension = 0;
        ref->setDomain(domain);
        ref->setNumberOfElements(numberOfElements.toInt());
        QStringList text;
        for (int idx=6+dimension.toInt(); idx< recordList.size(); idx++)
            text << recordList.at(idx);
        ref->setExplanatoryText(text.join(' '));
    }

    if (idx.toInt()!=size)
        return false;

    return true;

}

void ReferenceViewer::addReferenceInfo(SymbolReferenceItem *ref, const QString &referenceType, int lineNumber, int columnNumber, const QString &location)
{
    if (QString::compare(referenceType, "declared", Qt::CaseInsensitive)==0) {
        ref->addDeclare(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "defined", Qt::CaseInsensitive)==0) {
        ref->addDefine(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "assigned", Qt::CaseInsensitive)==0) {
        ref->addAssign(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "impl-assign", Qt::CaseInsensitive)==0) {
        ref->addImplicitAssign(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "ref", Qt::CaseInsensitive)==0) {
        ref->addReference(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "control", Qt::CaseInsensitive)==0) {
        ref->addControl(new ReferenceItem(location, lineNumber, columnNumber));
    } else if (QString::compare(referenceType, "index", Qt::CaseInsensitive)==0) {
        ref->addIndex(new ReferenceItem(location, lineNumber, columnNumber));
    }
}

QList<SymbolReferenceItem *> ReferenceViewer::findReference(SymbolDataType::SymbolType type)
{
    QList<SymbolReferenceItem*> refList;
    if (mValid) {
        QMap<SymbolId, SymbolReferenceItem*>::const_iterator i = mReference.constBegin();
        while(i != mReference.constEnd()) {
            SymbolReferenceItem* ref = i.value();
            if (type == ref->type())
                refList.append(ref);
            ++i;
        }
    }
    return refList;
}

} // namespace studio
} // namespace gams
