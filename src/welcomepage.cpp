/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "abstractview.h"
#include "welcomepage.h"
#include "commonpaths.h"
#include "settings.h"
#include "ui_welcomepage.h"
#include "mainwindow.h"
#include "wplabel.h"
#include "theme.h"
#include "file/fileicon.h"

namespace gams {
namespace studio {

WelcomePage::WelcomePage(MainWindow *parent)
    : AbstractView(parent)
    , ui(new Ui::WelcomePage)
    , mMain(parent)
{
    ui->setupUi(this);
    historyChanged();
    mOutputVisible = mMain->outputViewVisibility();

    QString path = CommonPaths::documentationDir() + "/";
    QString docs = ui->label_doc_studio->property("documentation").toString();
    ui->label_doc_studio->setProperty("documentation", path + docs);
    docs = ui->label_doc_release->property("documentation").toString();
    ui->label_doc_release->setProperty("documentation", path + docs);
    docs = ui->label_doc_tut->property("documentation").toString();
    ui->label_doc_tut->setProperty("documentation", path + docs);

    setupIcons();

    auto p = palette();
    p.setColor(QPalette::Window, p.color(QPalette::Base).lighter());

    connect(this, &WelcomePage::relayActionWp, parent, &MainWindow::receiveAction);
    connect(this, &WelcomePage::relayModLibLoad, parent, &MainWindow::receiveModLibLoad);
    connect(this, &WelcomePage::relayDocOpen, parent, &MainWindow::receiveOpenDoc);
    connect(this, &WelcomePage::zoomRequest, this, &WelcomePage::handleZoom);
}

void WelcomePage::historyChanged()
{
    QLayoutItem* item;
    while ((item = ui->layout_lastFiles->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    while ((item = ui->layout_lastProjects->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    WpLabel *tmpLabel;
    for (int isFile = 0; isFile < 2; ++isFile) {
        const QStringList &history = isFile ? mMain->history().files() : mMain->history().projects();
        int j = 0;
        for (int i = 0; i < Settings::settings()->toInt(skHistorySize); i++) {
            if (i >= history.size()) break;
            if (history.at(i) == "") continue;

            QFileInfo file(history.at(i));
            if (file.exists()) {
                tmpLabel = new WpLabel("<b>" + file.fileName() + "</b><br/>"
                                           + "<small>" + file.filePath() + "</small>", file.filePath(), this);
                tmpLabel->setCloseable();
                tmpLabel->setToolTip(file.filePath());
                tmpLabel->setIconSize(QSize(16,16));
                tmpLabel->setIcon(FileIcon::iconForFileKind(FileType::from(file.fileName()).kind()));
                connect(tmpLabel, &WpLabel::removeFromHistory, this, &WelcomePage::removeFromHistory);
                if (isFile) {
                    connect(tmpLabel, &QLabel::linkActivated, this, &WelcomePage::openFilePath);
                    ui->layout_lastFiles->addWidget(tmpLabel);
                } else {
                    connect(tmpLabel, &QLabel::linkActivated, this, &WelcomePage::openProject);
                    ui->layout_lastProjects->addWidget(tmpLabel);
                }
                j++;
            }
        }
        if (j == 0) {
            tmpLabel = new WpLabel(QString("<b>No recent files.</b><br/>"
                                           "<small>Start using GAMS Studio to populate this list.</small>"), "", this);
            if (isFile)
                ui->layout_lastFiles->addWidget(tmpLabel);
            else
                ui->layout_lastProjects->addWidget(tmpLabel);
        }
    }
}

WelcomePage::~WelcomePage()
{
    delete ui;
}

void WelcomePage::zoomReset()
{
    if (!parentWidget()) return;
    setFont(parentWidget()->font());
}

bool WelcomePage::event(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        auto p = qApp->palette();
        p.setColor(QPalette::Window, p.color(QPalette::Base).lighter());

        const auto laList = findChildren<WpLabel*>();
        for (WpLabel* w : laList)
            w->setPalette(p);
    }
    return AbstractView::event(event);
}

void WelcomePage::on_relayAction(const QString &action)
{
    emit relayActionWp(action);
}

void WelcomePage::on_relayModLibLoad(const QString &lib)
{
    emit relayModLibLoad(lib, false);
}

void WelcomePage::on_relayOpenDoc(const QString &doc, const QString &anchor)
{
    emit relayDocOpen(doc, anchor);
}

void WelcomePage::handleZoom(int delta)
{
    zoomInF(delta);
}

void WelcomePage::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    mOutputVisible = mMain->outputViewVisibility();
    mMain->setOutputViewVisibility(false);
    mMain->setHelpViewVisibility(false);
    historyChanged();
}

void WelcomePage::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    mMain->setOutputViewVisibility(mOutputVisible);
}

void WelcomePage::setupIcons()
{
    QSize size(16,16);
    ui->label_newfile->setIconSize(size);
    ui->label_newfile->setIcon(Theme::icon(":/%1/file"));
    ui->label_browseLib->setIndent(30);
    ui->label_browseLib->setIconSize(size);
    ui->label_browseLib->setIcon(Theme::icon(":/%1/books"));
    ui->label_trnsport->setIndent(30);
    ui->label_trnsport->setIconSize(size);
    ui->label_trnsport->setIcon(Theme::icon(":/%1/truck"));
    ui->label_doc_studio->setIndent(30);
    ui->label_doc_studio->setIconSize(size);
    ui->label_doc_studio->setIcon(Theme::icon(":/img/gams-w"));
    ui->label_doc_tut->setIndent(30);
    ui->label_doc_tut->setIconSize(size);
    ui->label_doc_tut->setIcon(Theme::icon(":/%1/book"));

    ui->label_whatsnew->setIndent(30);
    ui->label_whatsnew->setIconSize(size);
    ui->label_whatsnew->setIcon(Theme::icon(":/%1/new"));
    ui->label_doc_release->setIndent(30);
    ui->label_doc_release->setIconSize(size);
    ui->label_doc_release->setIcon(Theme::icon(":/%1/scroll"));
    ui->label_gamsworld->setIndent(30);
    ui->label_gamsworld->setIconSize(size);
    ui->label_gamsworld->setIcon(Theme::icon(":/img/gams-w"));
    ui->label_contact->setIndent(30);
    ui->label_contact->setIconSize(size);
    ui->label_contact->setIcon(Theme::icon(":/%1/envelope"));
}

}
}
