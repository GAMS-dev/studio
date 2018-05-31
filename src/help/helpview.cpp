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
#include <QToolBar>
#include <QDir>

#include "helpview.h"
#include "ui_helpview.h"

#include "checkforupdatewrapper.h"
#include "commonpaths.h"
#include "gclgms.h"

namespace gams {
namespace studio {

const QString HelpView::START_CHAPTER = "docs/index.html";
const QString HelpView::DOLLARCONTROL_CHAPTER = "docs/UG_DollarControlOptions.html";
const QString HelpView::GAMSCALL_CHAPTER = "docs/UG_GamsCall.html";
const QString HelpView::INDEX_CHAPTER = "docs/keyword.html";
const QString HelpView::OPTION_CHAPTER = "docs/UG_OptionStatement.html";
const QString HelpView::LATEST_ONLINE_HELP_URL = "https://www.gams.com/latest";

HelpView::HelpView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HelpView)
{
    CheckForUpdateWrapper c4uWrapper;
    mThisRelease = c4uWrapper.currentDistribVersionShort();
    mLastRelease = c4uWrapper.lastDistribVersionShort();

    if (c4uWrapper.distribIsLatest())
        onlineStartPageUrl = QUrl(LATEST_ONLINE_HELP_URL);
    else
        onlineStartPageUrl = QUrl( QString("https://www.gams.com/%1").arg(mThisRelease) );

    QDir dir = QDir(CommonPaths::systemDir()).filePath(START_CHAPTER);
    baseLocation = QDir(CommonPaths::systemDir()).absolutePath();
    startPageUrl = QUrl::fromLocalFile(dir.absolutePath());
    mOfflineHelpAvailable = (!dir.canonicalPath().isEmpty() && QFileInfo::exists(dir.canonicalPath()));

    mChapters << START_CHAPTER << DOLLARCONTROL_CHAPTER << GAMSCALL_CHAPTER
              << INDEX_CHAPTER << OPTION_CHAPTER;


    ui->setupUi(this);

    QToolBar* toolbar = new QToolBar(this);

    ui->actionHome->setToolTip("Start page ("+ QDir(CommonPaths::systemDir()).filePath(START_CHAPTER)+")");
    ui->actionHome->setStatusTip("Start page ("+ QDir(CommonPaths::systemDir()).filePath(START_CHAPTER)+")");
    connect(ui->actionHome, &QAction::triggered, this, &HelpView::on_actionHome_triggered);

    toolbar->addAction(ui->actionHome);
    toolbar->addSeparator();
    toolbar->addAction(ui->webEngineView->pageAction(QWebEnginePage::Back));
    toolbar->addAction(ui->webEngineView->pageAction(QWebEnginePage::Forward));
    toolbar->addSeparator();
    toolbar->addAction(ui->webEngineView->pageAction(QWebEnginePage::Reload));
    toolbar->addSeparator();
    toolbar->addAction(ui->webEngineView->pageAction(QWebEnginePage::Stop));
    toolbar->addSeparator();

    ui->toolbarVlLayout->addWidget(toolbar);

    if (mOfflineHelpAvailable) {
        ui->webEngineView->load(startPageUrl);
    } else {
        QString htmlText;
        getErrorHTMLText( htmlText, START_CHAPTER);
        ui->webEngineView->setHtml( htmlText );
    }
    connect(ui->webEngineView, &QWebEngineView::loadFinished, this, &HelpView::on_loadFinished);
}

HelpView::~HelpView()
{
    delete ui;
}

void HelpView::on_actionHome_triggered()
{
    ui->webEngineView->load(startPageUrl);
}

void HelpView::on_loadFinished(bool ok)
{
    ui->actionOnlineHelp->setEnabled( true );
    ui->actionOnlineHelp->setChecked( false );
    if (ok) {
//       if (ui->webEngineView->url().host().compare("www.gams.com", Qt::CaseInsensitive) == 0 ) {
//           if (ui->webEngineView->url().path().contains(mThisRelease))
//               ui->actionOnlineHelp->setChecked( true );
//           else if (mHelpView->url().path().contains("latest") && (mThisRelease == mLastRelease))
//               ui->actionOnlineHelp->setChecked( true );
//           else
//               ui->actionOnlineHelp->setEnabled( false );

//       } else {
//           if (ui->webEngineView->url().scheme().compare("file", Qt::CaseSensitive) !=0 )
//               ui->actionOnlineHelp->setEnabled( false );
//       }
    }
}

void HelpView::getErrorHTMLText(QString &htmlText, const QString &chapterText)
{
    QString downloadPage = QString("https://www.gams.com/%1").arg(mThisRelease);

    htmlText = "<html><head><title>Error Loading Help</title></head><body>";
    htmlText += "<div id='message'>Help Document Not Found from expected GAMS Installation at ";
    htmlText += QDir(CommonPaths::systemDir()).filePath(chapterText);
    htmlText += "</div><br/> <div>Please check your GAMS installation and configuration. You can reinstall GAMS from <a href='";
    htmlText += downloadPage;
    htmlText += "'>";
    htmlText += downloadPage;
    htmlText += "</a> or from the latest download page <a href='https://www.gams.com/latest'>https://www.gams.com/latest</a>.</div> </body></html>";
}

} // namespace studio
} // namespace gams
