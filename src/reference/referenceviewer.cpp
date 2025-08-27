/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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

ReferenceViewer::ReferenceViewer(const QString &referenceFile, const QString &encodingName, QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::ReferenceViewer),
    mEncodingName(encodingName),
    mReference(new Reference(referenceFile, encodingName))
{
    ui->setupUi(this);
    updateStyle();

    bool problemLoaded = (mReference->state() == Reference::Loading ||
                          mReference->state() == Reference::UnsuccessfullyLoaded );

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
    ui->tabWidget->addTab(fileusedRefWidget, QString("File Used (%1)").arg( problemLoaded ? "?" : "..."));
    headers << fileusedRefWidget->headers();

    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setEnabled(!problemLoaded);
    allSymbolsRefWidget->initModel();
    setFocusProxy(ui->tabWidget);

    for (QHeaderView *header : std::as_const(headers)) {
        headerRegister(header);
    }

    connect(ui->tabWidget, &QTabWidget::tabBarClicked, this, &ReferenceViewer::on_tabBarClicked);
    connect(mReference.data(), &Reference::reloadFiledUsedTabFinished, this, &ReferenceViewer::updateFileUsedTabText);
    connect(mReference.data(), &Reference::loadFinished, this, &ReferenceViewer::updateView);
    if (problemLoaded) {
        // call loadReferenceFile() again every 500 ms
        QTimer::singleShot(500, this, [this](){ mReference->loadReferenceFile(mEncodingName, true); });
    }
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
    SymbolReferenceWidget* tabWidget = qobject_cast<SymbolReferenceWidget*>(ui->tabWidget->currentWidget());
    if (tabWidget)
        tabWidget->selectSearchField();
}

void ReferenceViewer::reloadFile(const QString &encodingName)
{
    FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->tabWidget->widget(11));
    if (fileUsedWidget) {
        fileUsedWidget->deActivateFilter();
    }
    mEncodingName = encodingName;
    mReference->loadReferenceFile(mEncodingName, true);
}

void ReferenceViewer::on_tabBarClicked(int index)
{
    SymbolReferenceWidget* refWidget = toSymbolReferenceWidget(ui->tabWidget->widget(index));
    if (refWidget) {
        if (!refWidget->isModelLoaded())
            refWidget->initModel();
        refWidget->setFocus();
    } else {
        FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->tabWidget->widget(index));
        if (fileUsedWidget) {
            if (!fileUsedWidget->isModelLoaded()) {
               fileUsedWidget->activateFilter();
               fileUsedWidget->initModel();
            }
            fileUsedWidget->setFocus();
        }
    }
}

void ReferenceViewer::updateView(bool loadStatus, bool pendingReload)
{
    if (loadStatus && !pendingReload) { // SuccessfullyLoaded and no pending relaad
        for(int i=0; i<ui->tabWidget->count(); i++) {
            SymbolReferenceWidget* refWidget = toSymbolReferenceWidget(ui->tabWidget->widget(i));
            if (refWidget) {
                refWidget->initModel(mReference.data());
                refWidget->resetModel();
            } else {
                FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->tabWidget->widget(i));
                if (fileUsedWidget) {
                    fileUsedWidget->activateFilter();
                    fileUsedWidget->initModel(mReference.data());
                    fileUsedWidget->resetModel();
                }
            }
        }

        ui->tabWidget->setToolTip(QString(""));
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
        FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->tabWidget->widget(11));
        if (fileUsedWidget)
            ui->tabWidget->setTabText(11, QString("File Used (%1)").arg(mReference->getNumberOfFileUsed(fileUsedWidget->isViewCompact())));
        else
            ui->tabWidget->setTabText(11, QString("File Used (...)"));
        ui->tabWidget->setEnabled(true);
    } else {
        QString errorLine = (mReference->errorLine() > 0 ? QString(":%1").arg(mReference->errorLine()) : "");
        ui->tabWidget->setToolTip(QString("<p style='white-space:pre'>Error while loading: %1%2<br>The file content might be corrupted or incorrectly overwritten</p>")
                                      .arg(mReference->getFileLocation()).arg(errorLine));
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
        ui->tabWidget->setEnabled(false);
    }

    if (pendingReload) {
        // call loadReferenceFile() again every 500 ms
        QTimer::singleShot(500, this, [this](){ mReference->loadReferenceFile(mEncodingName, true); });
    } else { // no reload pending
        QProcess::ProcessState state = QProcess::ProcessState::NotRunning;
        emit processState(state);
        if (state == QProcess::ProcessState::NotRunning &&
            mReference->state() == Reference::UnsuccessfullyLoaded ) {
                QString errorLine = (mReference->errorLine() > 0 ? QString(":%1").arg(mReference->errorLine()) : "");
                SysLogLocator::systemLog()->append(
                    QString("Error while loading: %1%2, the file content might be corrupted or incorrectly overwritten")
                        .arg(mReference->getFileLocation(), errorLine),
                        LogMsgType::Error);
        }
    }
}

void ReferenceViewer::updateFileUsedTabText(bool compactview)
{
    ui->tabWidget->setTabText(11, QString("File Used (%1)").arg(mReference->getNumberOfFileUsed(compactview)));
}

int ReferenceViewer::currentSelectedTab()
{
    return ui->tabWidget->currentIndex();
}

ReferenceSettings ReferenceViewer::saveSettings()
{
    int tabIndex = currentSelectedTab();
    SymbolReferenceWidget* refWidget = toSymbolReferenceWidget(ui->tabWidget->widget(tabIndex));
    if (refWidget) {
        return ReferenceSettings(tabIndex, refWidget->currentReferenceItem(), -1, false);
    } else {
        FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->tabWidget->widget(tabIndex));
        if (fileUsedWidget) {
            return ReferenceSettings(tabIndex, fileUsedWidget->referenceItem(fileUsedWidget->currentReferenceId()),
                                     fileUsedWidget->currentReferenceId(), fileUsedWidget->isViewCompact());
        }
    }
    return ReferenceSettings(tabIndex, ReferenceItem(),  -1, false);
}

void ReferenceViewer::loadSettings(const ReferenceSettings &settings)
{
    ui->tabWidget->setCurrentIndex(settings.tabIndex);
    on_tabBarClicked(ui->tabWidget->currentIndex());
    SymbolReferenceWidget* refWidget = toSymbolReferenceWidget(ui->tabWidget->widget(settings.tabIndex));
    if (refWidget) {
        refWidget->selectSymbolReference(settings.item);
    } else {
        FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->tabWidget->widget(settings.tabIndex));
        if (fileUsedWidget) {
            fileUsedWidget->selectFileReference(settings.id, settings.compactView);
        }
    }
}


void ReferenceViewer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Up:
        if (ui->tabWidget->currentIndex() > 0) {
            SymbolReferenceWidget* symRefWidget = toSymbolReferenceWidget( ui->tabWidget->widget( ui->tabWidget->currentIndex()-1 ) );
            if (symRefWidget) {
                ui->tabWidget->setCurrentWidget( symRefWidget );
                on_tabBarClicked( ui->tabWidget->currentIndex() );
                e->accept();
                return;
            } else {
                FileReferenceWidget* fileRefWidget = toFileUsedReferenceWidget( ui->tabWidget->widget( ui->tabWidget->currentIndex()-1 ) );
                if (fileRefWidget) {
                    ui->tabWidget->setCurrentWidget( fileRefWidget );
                    on_tabBarClicked( ui->tabWidget->currentIndex() );
                    e->accept();
                    return;
                }
            }
        }
        break;
    case Qt::Key_Down: {
        if (ui->tabWidget->currentIndex() < ui->tabWidget->count()) {
            SymbolReferenceWidget* symRefWidget = toSymbolReferenceWidget( ui->tabWidget->widget( ui->tabWidget->currentIndex()+1 ) );
            if (symRefWidget) {
                ui->tabWidget->setCurrentWidget( symRefWidget );
                on_tabBarClicked( ui->tabWidget->currentIndex() );
                e->accept();
                return;
            } else {
                FileReferenceWidget* fileRefWidget = toFileUsedReferenceWidget( ui->tabWidget->widget( ui->tabWidget->currentIndex()+1 ) );
                if (fileRefWidget) {
                    ui->tabWidget->setCurrentWidget( fileRefWidget );
                    on_tabBarClicked( ui->tabWidget->currentIndex() );
                    e->accept();
                    return;
                }
            }
        }
        break;
    }
    }
    QWidget::keyPressEvent(e);
}

} // namespace reference
} // namespace studio
} // namespace gams
