/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include <QStackedWidget>
#include <QWheelEvent>
#include <QStandardItemModel>

#include "referenceviewer.h"
#include "ui_referenceviewer.h"
#include "filereferencewidget.h"
#include "symbolreferenceitem.h"
#include "symbolreferencewidget.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include "theme.h"

namespace gams {
namespace studio {
namespace reference {

const int CExtraMargin = 12;

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
    while (ui->stackedWidget->count())
        ui->stackedWidget->removeWidget(ui->stackedWidget->widget(0));

    QStringList items;
    if (problemLoaded) {
        items << QString("All Symbols (?)")
              << QString("Set (?)")
              << QString("Acronym (?)")
              << QString("Variable (?)")
              << QString("Parameter (?)")
              << QString("Equation (?)")
              << QString("Model (?)")
              << QString("File (?)")
              << QString("Macro (?)")
              << QString("Function (?)")
              << QString("Unused (?)")
              << QString("File Used (?)");
    } else {
        items << QString("All Symbols (%1)").arg(mReference->size())
              << QString("Set (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Set).size())
              << QString("Acronym (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Acronym).size())
              << QString("Variable (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Variable).size())
              << QString("Parameter (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Parameter).size())
              << QString("Equation (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Equation).size())
              << QString("Model (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Model).size())
              << QString("File (%1)").arg(mReference->findReferenceFromType(SymbolDataType::File).size())
              << QString("Macro (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Macro).size())
              << QString("Function (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Funct).size())
              << QString("Unused (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Unused).size())
              << QString("File Used (%1)").arg("...");
    }
    mNavModel = new QStandardItemModel(0, 1, this);
    int itemHeight = int(ui->listView->fontMetrics().height() * 1.8);
    for (int i = 0; i < items.count(); ++i) {
        QStandardItem *item = new QStandardItem();
        item->setData(items.at(i), Qt::DisplayRole);
        item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
        item->setData(QSize(0, itemHeight), Qt::SizeHintRole);
        mNavModel->appendRow(item);
    }

    ui->listView->setModel(mNavModel);
    // ui->comboBox->setModel(mNavModel);   // TODO(JM) prepared for later enhancement
    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex &current) {
        ui->stackedWidget->setCurrentIndex(current.row());
        // ui->comboBox->setCurrentIndex(current.row());  // TODO(JM) prepared for later enhancement
    });

    // TODO(JM) prepared for later enhancement
    // connect(ui->comboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
    //     ui->stackedWidget->setCurrentIndex(index);
    //     ui->listView->setCurrentIndex(mNavModel->index(index, 0));
    // });

    SymbolReferenceWidget* allSymbolsRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Unknown, this));
    ui->stackedWidget->addWidget(allSymbolsRefWidget);
    headers << allSymbolsRefWidget->headers();

    SymbolReferenceWidget* setRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Set, this));
    ui->stackedWidget->addWidget(setRefWidget);
    headers << setRefWidget->headers();

    SymbolReferenceWidget* acronymRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Acronym, this));
    ui->stackedWidget->addWidget(acronymRefWidget);
    headers << acronymRefWidget->headers();

    SymbolReferenceWidget* varRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Variable, this));
    ui->stackedWidget->addWidget(varRefWidget);
    headers << varRefWidget->headers();

    SymbolReferenceWidget* parRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Parameter, this));
    ui->stackedWidget->addWidget(parRefWidget);
    headers << parRefWidget->headers();

    SymbolReferenceWidget* equRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Equation, this));
    ui->stackedWidget->addWidget(equRefWidget);
    headers << equRefWidget->headers();

    SymbolReferenceWidget* modelRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Model, this));
    ui->stackedWidget->addWidget(modelRefWidget);
    headers << modelRefWidget->headers();

    SymbolReferenceWidget* fileRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::File, this));
    ui->stackedWidget->addWidget(fileRefWidget);
    headers << fileRefWidget->headers();

    SymbolReferenceWidget* macroRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Macro, this));
    ui->stackedWidget->addWidget(macroRefWidget);
    headers << macroRefWidget->headers();

    SymbolReferenceWidget* functRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Funct, this));
    ui->stackedWidget->addWidget(functRefWidget);
    headers << functRefWidget->headers();

    SymbolReferenceWidget* unusedRefWidget = initViewerType(new SymbolReferenceWidget(mReference.data(), SymbolDataType::Unused, this));
    ui->stackedWidget->addWidget(unusedRefWidget);
    headers << unusedRefWidget->headers();

    FileReferenceWidget* fileusedRefWidget = initViewerType(new FileReferenceWidget(mReference.data(), this));
    ui->stackedWidget->addWidget(fileusedRefWidget);
    headers << fileusedRefWidget->headers();

    ui->listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listView->installEventFilter(this);
    ui->listView->viewport()->installEventFilter(this);
    setCurrentViewerIndex(0);
    ui->stackedWidget->setEnabled(!problemLoaded);
    allSymbolsRefWidget->initModel();
    setFocusProxy(ui->stackedWidget);
    updateTabs();

    for (QHeaderView *header : std::as_const(headers)) {
        headerRegister(header);
    }

    connect(mReference.data(), &Reference::reloadFiledUsedTabFinished, this, &ReferenceViewer::updateFileUsedTabText);
    connect(mReference.data(), &Reference::loadFinished, this, &ReferenceViewer::updateView);
    if (problemLoaded) {
        // call loadReferenceFile() again every 500 ms
        QTimer::singleShot(500, this, [this](){ mReference->loadReferenceFile(mEncodingName, true); });
    }
    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex &current) {
        if (current.isValid()) setCurrentViewerIndex(current.row());
    });

    // TODO(JM) prepared for later enhancement
    // connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &ReferenceViewer::setCurrentViewerIndex);
}

ReferenceViewer::~ReferenceViewer()
{
    delete ui;
}

void ReferenceViewer::updateStyle()
{
    Theme::setThemeColorPalette(ui->stackedWidget, true, false);
    updateTabs();
}

bool ReferenceViewer::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->listView->viewport()) {
        if (event->type() == QEvent::MouseMove || event->type() == QEvent::Leave) {
            QModelIndex hoveredIndex;
            if (event->type() == QEvent::MouseMove) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                hoveredIndex = ui->listView->indexAt(mouseEvent->pos());

            }
            for (int i = 0; i < mNavModel->rowCount(); ++i) {
                if (QStandardItem *item = mNavModel->item(i, 0)) {
                    item->setData(hoveredIndex.isValid() && i == hoveredIndex.row()
                                  ? qApp->palette().brush(QPalette::AlternateBase)
                                  : QVariant(), Qt::BackgroundRole);
                }
            }
        }
    } else if (watched == ui->listView && event->type() == QEvent::FontChange) {
        updateTabs();
    }
    return AbstractView::eventFilter(watched, event);
}

void ReferenceViewer::selectSearchField() const
{
    SymbolReferenceWidget* stackedWidget = qobject_cast<SymbolReferenceWidget*>(ui->stackedWidget->currentWidget());
    if (stackedWidget)
        stackedWidget->selectSearchField();
}

void ReferenceViewer::reloadFile(const QString &encodingName)
{
    FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->stackedWidget->widget(11));
    if (fileUsedWidget) {
        fileUsedWidget->deActivateFilter();
    }
    mEncodingName = encodingName;
    mReference->loadReferenceFile(mEncodingName, true);
}

void ReferenceViewer::setCurrentViewerIndex(int index)
{
    if (index < 0 || index >= mNavModel->rowCount()) return;

    if (ui->listView->currentIndex().row() != index) {
        QSignalBlocker blocker(ui->listView->selectionModel());
        ui->listView->setCurrentIndex(mNavModel->index(index, 0));
    }

    // if (ui->comboBox->currentIndex() != index) {
    //     QSignalBlocker blocker(ui->comboBox);
    //     ui->comboBox->setCurrentIndex(index);
    // }

    ui->stackedWidget->setCurrentIndex(index);
    selectStackedIndex(index);
}

void ReferenceViewer::selectStackedIndex(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
    SymbolReferenceWidget* refWidget = toSymbolReferenceWidget(ui->stackedWidget->widget(index));
    if (refWidget) {
        if (!refWidget->isModelLoaded())
            refWidget->initModel();
        refWidget->setFocus();
    } else {
        FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->stackedWidget->widget(index));
        if (fileUsedWidget) {
            if (!fileUsedWidget->isModelLoaded()) {
                fileUsedWidget->activateFilter();
                fileUsedWidget->initModel();
            }
            fileUsedWidget->setFocus();
        }
    }
}

void ReferenceViewer::updateTabs()
{
    if (!mNavModel) return;
    QFontMetrics fm = ui->listView->fontMetrics();
    int itemHeight = int(ui->listView->fontMetrics().height() * 1.8);
    int longestItemWidth = 0;
    for (int i = 0; i < mNavModel->rowCount(); ++i) {
        QModelIndex index = mNavModel->index(i, 0);
        longestItemWidth = qMax(longestItemWidth, fm.horizontalAdvance(mNavModel->data(index).toString()));
        bool enabled = !mNavModel->data(index).toString().endsWith("(0)")
                       && !mNavModel->data(index).toString().endsWith("(?)");
        QColor textColor = enabled ? qApp->palette().color(QPalette::Active, QPalette::Text)
                                   : qApp->palette().color(QPalette::Disabled, QPalette::Text);
        mNavModel->setData(index, textColor, Qt::ForegroundRole);
        mNavModel->setData(index, QSize(0, itemHeight), Qt::SizeHintRole);
        Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        if (!enabled)
            flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        mNavModel->setItemData(index, {{Qt::UserRole, QVariant(int(flags))}});
    }
    int frameWidth = (ui->listView->frameWidth() + CExtraMargin) * 2;
    ui->listView->setFixedWidth(longestItemWidth + frameWidth);
}

void ReferenceViewer::updateView(bool loadStatus, bool pendingReload)
{
    if (loadStatus && !pendingReload) { // SuccessfullyLoaded and no pending relaad
        for(int i=0; i<ui->stackedWidget->count(); i++) {
            SymbolReferenceWidget* refWidget = toSymbolReferenceWidget(ui->stackedWidget->widget(i));
            if (refWidget) {
                refWidget->initModel(mReference.data());
                refWidget->resetModel();
            } else {
                FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->stackedWidget->widget(i));
                if (fileUsedWidget) {
                    fileUsedWidget->activateFilter();
                    fileUsedWidget->initModel(mReference.data());
                    fileUsedWidget->resetModel();
                }
            }
        }

        ui->listView->setToolTip(QString(""));
        mNavModel->setData(mNavModel->index(0, 0), QString("All Symbols (%1)").arg(mReference->size()));
        mNavModel->setData(mNavModel->index(1, 0), QString("Set (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Set).size()));
        mNavModel->setData(mNavModel->index(2, 0), QString("Acronym (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Acronym).size()));
        mNavModel->setData(mNavModel->index(3, 0), QString("Variable (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Variable).size()));
        mNavModel->setData(mNavModel->index(4, 0), QString("Parameter (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Parameter).size()));
        mNavModel->setData(mNavModel->index(5, 0), QString("Equation (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Equation).size()));
        mNavModel->setData(mNavModel->index(6, 0), QString("Model (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Model).size()));
        mNavModel->setData(mNavModel->index(7, 0), QString("File (%1)").arg(mReference->findReferenceFromType(SymbolDataType::File).size()));
        mNavModel->setData(mNavModel->index(8, 0), QString("Macro (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Macro).size()));
        mNavModel->setData(mNavModel->index(9, 0), QString("Function (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Funct).size()));
        mNavModel->setData(mNavModel->index(10, 0), QString("Unused (%1)").arg(mReference->findReferenceFromType(SymbolDataType::Unused).size()));
        FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->stackedWidget->widget(11));
        if (fileUsedWidget)
            mNavModel->setData(mNavModel->index(11, 0), QString("File Used (%1)").arg(mReference->getNumberOfFileUsed(fileUsedWidget->isViewCompact())));
        else
            mNavModel->setData(mNavModel->index(11, 0), QString("File Used (...)"));
        ui->stackedWidget->setEnabled(true);
    } else {
        QString errorLine = (mReference->errorLine() > 0 ? QString(":%1").arg(mReference->errorLine()) : "");
        ui->stackedWidget->setToolTip(QString("<p style='white-space:pre'>Error while loading: %1%2<br>The file content might be corrupted or incorrectly overwritten</p>")
                                      .arg(mReference->getFileLocation(), errorLine));
        mNavModel->setData(mNavModel->index(0, 0), QString("All Symbols (?)"));
        mNavModel->setData(mNavModel->index(1, 0), QString("Set (?)"));
        mNavModel->setData(mNavModel->index(2, 0), QString("Acronym (?)"));
        mNavModel->setData(mNavModel->index(3, 0), QString("Variable (?)"));
        mNavModel->setData(mNavModel->index(4, 0), QString("Parameter (?)"));
        mNavModel->setData(mNavModel->index(5, 0), QString("Equation (?)"));
        mNavModel->setData(mNavModel->index(6, 0), QString("Model (?)"));
        mNavModel->setData(mNavModel->index(7, 0), QString("File (?)"));
        mNavModel->setData(mNavModel->index(8, 0), QString("Macro (?)"));
        mNavModel->setData(mNavModel->index(9, 0), QString("Function (?)"));
        mNavModel->setData(mNavModel->index(10, 0), QString("Unused (?)"));
        mNavModel->setData(mNavModel->index(11, 0), QString("File Used (?)"));
        ui->stackedWidget->setCurrentIndex(0);
        ui->stackedWidget->setEnabled(false);
    }
    updateTabs();

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
    mNavModel->setData(mNavModel->index(11, 0), QString("File Used (%1)").arg(mReference->getNumberOfFileUsed(compactview)));
}

int ReferenceViewer::currentSelectedTab()
{
    return ui->stackedWidget->currentIndex();
}

ReferenceSettings ReferenceViewer::saveSettings()
{
    int tabIndex = currentSelectedTab();
    SymbolReferenceWidget* refWidget = toSymbolReferenceWidget(ui->stackedWidget->widget(tabIndex));
    if (refWidget) {
        return ReferenceSettings(tabIndex, refWidget->currentReferenceItem(), -1, false);
    } else {
        FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->stackedWidget->widget(tabIndex));
        if (fileUsedWidget) {
            return ReferenceSettings(tabIndex, fileUsedWidget->referenceItem(fileUsedWidget->currentReferenceId()),
                                     fileUsedWidget->currentReferenceId(), fileUsedWidget->isViewCompact());
        }
    }
    return ReferenceSettings(tabIndex, ReferenceItem(),  -1, false);
}


void ReferenceViewer::loadSettings(const ReferenceSettings &settings)
{
    ui->stackedWidget->setCurrentIndex(settings.tabIndex);
    selectStackedIndex(ui->stackedWidget->currentIndex());
    SymbolReferenceWidget* refWidget = toSymbolReferenceWidget(ui->stackedWidget->widget(settings.tabIndex));
    if (refWidget) {
        refWidget->selectSymbolReference(settings.item);
    } else {
        FileReferenceWidget* fileUsedWidget = toFileUsedReferenceWidget(ui->stackedWidget->widget(settings.tabIndex));
        if (fileUsedWidget) {
            fileUsedWidget->selectFileReference(settings.id, settings.compactView);
        }
    }
}



void ReferenceViewer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Up:
        if (ui->stackedWidget->currentIndex() > 0) {
            SymbolReferenceWidget* symRefWidget = toSymbolReferenceWidget( ui->stackedWidget->widget( ui->stackedWidget->currentIndex()-1 ) );
            if (symRefWidget) {
                ui->stackedWidget->setCurrentWidget( symRefWidget );
                selectStackedIndex( ui->stackedWidget->currentIndex() );
                e->accept();
                return;
            } else {
                FileReferenceWidget* fileRefWidget = toFileUsedReferenceWidget( ui->stackedWidget->widget( ui->stackedWidget->currentIndex()-1 ) );
                if (fileRefWidget) {
                    ui->stackedWidget->setCurrentWidget( fileRefWidget );
                    selectStackedIndex( ui->stackedWidget->currentIndex() );
                    e->accept();
                    return;
                }
            }
        }
        break;
    case Qt::Key_Down: {
        if (ui->stackedWidget->currentIndex() < ui->stackedWidget->count()) {
            SymbolReferenceWidget* symRefWidget = toSymbolReferenceWidget( ui->stackedWidget->widget( ui->stackedWidget->currentIndex()+1 ) );
            if (symRefWidget) {
                ui->stackedWidget->setCurrentWidget( symRefWidget );
                selectStackedIndex( ui->stackedWidget->currentIndex() );
                e->accept();
                return;
            } else {
                FileReferenceWidget* fileRefWidget = toFileUsedReferenceWidget( ui->stackedWidget->widget( ui->stackedWidget->currentIndex()+1 ) );
                if (fileRefWidget) {
                    ui->stackedWidget->setCurrentWidget( fileRefWidget );
                    selectStackedIndex( ui->stackedWidget->currentIndex() );
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
