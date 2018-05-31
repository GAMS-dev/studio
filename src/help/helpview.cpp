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
#include <QCheckBox>
#include <QDir>
#include <QDesktopServices>
#include <QKeyEvent>
#include <QToolBar>
#include <QToolButton>

#include "helpview.h"
#include "ui_helpview.h"

#include "bookmarkdialog.h"
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

    toolbar->addAction(ui->actionHome);
    toolbar->addSeparator();
    toolbar->addAction(ui->webEngineView->pageAction(QWebEnginePage::Back));
    toolbar->addAction(ui->webEngineView->pageAction(QWebEnginePage::Forward));
    toolbar->addSeparator();
    toolbar->addAction(ui->webEngineView->pageAction(QWebEnginePage::Reload));
    toolbar->addSeparator();
    toolbar->addAction(ui->webEngineView->pageAction(QWebEnginePage::Stop));
    toolbar->addSeparator();

    mBookmarkMenu = new QMenu(this);
    mBookmarkMenu->addAction(ui->actionAddBookmark);
    mBookmarkMenu->addSeparator();
    mBookmarkMenu->addAction(ui->actionOrganizeBookmark);

    QToolButton* bookmarkToolButton = new QToolButton(this);
    QIcon bookmarkButtonIcon(":/img/bookmark");
    bookmarkToolButton->setToolTip("Bookmarks");
    bookmarkToolButton->setIcon(bookmarkButtonIcon);
    bookmarkToolButton->setIcon(bookmarkButtonIcon);
    bookmarkToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    bookmarkToolButton->setMenu(mBookmarkMenu);
    toolbar->addWidget(bookmarkToolButton);

    QWidget* spacerWidget = new QWidget();
    spacerWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    toolbar->addWidget(spacerWidget);

    QMenu* helpMenu = new QMenu;
    ui->actionOnlineHelp = new QAction("View This Page from https://www.gams.com/"+mThisRelease+"/", this);
    ui->actionOnlineHelp->setStatusTip("View This Page from https://www.gams.com/"+mThisRelease+"/");
    ui->actionOnlineHelp->setCheckable(true);
    helpMenu->addAction(ui->actionOnlineHelp);
    helpMenu->addSeparator();

    ui->actionOpenInBrowser->setStatusTip(tr("Open this page in Default Web Browser"));
    helpMenu->addAction(ui->actionOpenInBrowser);
    helpMenu->addSeparator();
    helpMenu->addAction(ui->actionCopyPageURL);

    ui->actionCopyPageURL->setStatusTip(tr("Copy URL of this page to Clipboard"));

    QToolButton* helpToolButton = new QToolButton(this);
    QIcon toolButtonIcon(":/img/config");
    helpToolButton->setToolTip("Help Option");
    helpToolButton->setIcon(toolButtonIcon);
    helpToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    helpToolButton->setMenu(helpMenu);
    toolbar->addWidget(helpToolButton);

    ui->toolbarVlLayout->addWidget(toolbar);

    if (mOfflineHelpAvailable) {
        ui->webEngineView->load(startPageUrl);
    } else {
        QString htmlText;
        getErrorHTMLText( htmlText, START_CHAPTER);
        ui->webEngineView->setHtml( htmlText );
    }
    connect(ui->webEngineView, &QWebEngineView::loadFinished, this, &HelpView::on_loadFinished);

    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &HelpView::searchText);
    connect(ui->backButton, &QPushButton::clicked, this, &HelpView::on_backButtonTriggered);
    connect(ui->forwardButton, &QPushButton::clicked, this, &HelpView::on_forwardButtonTriggered);
    connect(ui->caseSenstivity, &QCheckBox::clicked, this, &HelpView::on_caseSensitivityToggled);
    connect(ui->closeButton, &QPushButton::clicked, this, &HelpView::on_closeButtonTriggered);
}

HelpView::~HelpView()
{
    delete ui;
}

void HelpView::clearStatusBar()
{
    ui->searchLineEdit->clear();
    findText("", Forward);
    ui->statusbarWidget->hide();
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
       if (ui->webEngineView->url().host().compare("www.gams.com", Qt::CaseInsensitive) == 0 ) {
           if (ui->webEngineView->url().path().contains(mThisRelease))
               ui->actionOnlineHelp->setChecked( true );
           else if (ui->webEngineView->url().path().contains("latest") && (mThisRelease == mLastRelease))
               ui->actionOnlineHelp->setChecked( true );
           else
               ui->actionOnlineHelp->setEnabled( false );

       } else {
           if (ui->webEngineView->url().scheme().compare("file", Qt::CaseSensitive) !=0 )
               ui->actionOnlineHelp->setEnabled( false );
       }
    }
}

void HelpView::on_actionAddBookmark_triggered()
{
    if (mBookmarkMap.size() == 0)
        mBookmarkMenu->addSeparator();

    QString pageUrl = ui->webEngineView->page()->url().toString();
    bool found = false;
    foreach (QAction* action, mBookmarkMenu->actions()) {
        if (action->isSeparator())
            continue;
        if ((QString::compare(action->objectName(), pageUrl, Qt::CaseInsensitive) == 0) &&
            (QString::compare(action->text(),  ui->webEngineView->page()->title(), Qt::CaseInsensitive) == 0)
           ) {
              found = true;
              break;
        }
    }
    if (!found) {
       mBookmarkMap.replace(pageUrl, ui->webEngineView->page()->title());
       addBookmarkAction(pageUrl, ui->webEngineView->page()->title());
    }
}

void HelpView::on_actionOrganizeBookmark_triggered()
{
    BookmarkDialog bookmarkDialog(mBookmarkMap, this);
    bookmarkDialog.exec();
}

void HelpView::on_actionOnlineHelp_triggered(bool checked)
{
    QUrl url = ui->webEngineView->url();

    if (checked) {
        QString urlStr = url.toDisplayString();
        urlStr.replace( urlStr.indexOf("file://"), 7, "");
        urlStr.replace( urlStr.indexOf( baseLocation),
                        baseLocation.size(),
                        onlineStartPageUrl.toDisplayString() );
        url = QUrl(urlStr);
    } else {
        if (mOfflineHelpAvailable) {
            QString urlStr = url.toDisplayString();
            urlStr.replace( urlStr.indexOf( onlineStartPageUrl.toDisplayString() ),
                            onlineStartPageUrl.toDisplayString().size(),
                            baseLocation);
            url.setUrl(urlStr);
            url.setScheme("file");
        } else {
            QString htmlText;
            getErrorHTMLText( htmlText, "");
            ui->webEngineView->setHtml( htmlText );
        }
    }
    ui->actionOnlineHelp->setChecked( checked );
    ui->webEngineView->load( url );
}

void HelpView::on_actionOpenInBrowser_triggered()
{
    QDesktopServices::openUrl( ui->webEngineView->url() );
}

void HelpView::on_actionCopyPageURL_triggered()
{
    QClipboard* clip = QApplication::clipboard();;
    clip->setText( mHelpView->page()->url().toString());
}

void HelpView::on_actionBookmark_triggered()
{
    QAction* sAction = qobject_cast<QAction*>(sender());
    ui->webEngineView->load( QUrl(sAction->toolTip(), QUrl::TolerantMode) );
}

void HelpView::addBookmarkAction(const QString &objectName, const QString &title)
{
    QAction* action = new QAction(this);
    action->setObjectName(objectName);
    action->setText(title);
    action->setToolTip(objectName);

    if (objectName.startsWith("file")) {
           QIcon linkButtonIcon(":/img/link");
           action->setIcon(linkButtonIcon);
    } else if (objectName.startsWith("http")) {
           QIcon linkButtonIcon(":/img/external-link");
           action->setIcon(linkButtonIcon);
    }
    connect(action, &QAction::triggered, this, &HelpView::on_actionBookmark_triggered);
    mBookmarkMenu->addAction(action);
}

void HelpView::on_searchHelp()
{
    if (ui->webEngineView->isVisible()) {
        clearStatusBar();
        ui->webEngineView->setFocus();
    } else {
        ui->statusbarWidget->show();
        ui->webEngineView->setFocus();
    }
}

void HelpView::on_backButtonTriggered()
{
    findText(ui->searchLineEdit->text(), Backward);
}

void HelpView::on_forwardButtonTriggered()
{
    findText(ui->searchLineEdit->text(), Forward);
}

void HelpView::on_closeButtonTriggered()
{
    on_searchHelp();
}

void HelpView::on_caseSensitivityToggled(bool checked)
{
    findText("", Forward);
    findText(ui->searchLineEdit->text(), Forward);
}

void HelpView::searchText(const QString &text)
{
    findText(text, Forward);
}

void HelpView::closeEvent(QCloseEvent *event)
{
    clearStatusBar();
    QWidget::closeEvent(event);
}

void HelpView::keyPressEvent(QKeyEvent *event)
{
    if (ui->statusbarWidget->isVisible()) {
        if (event->key() == Qt::Key_Escape) {
           clearStatusBar();
           ui->webEngineView->setFocus();
        } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
                  on_forwardButtonTriggered();
        }
    }
    QWidget::keyPressEvent(event);
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

void HelpView::findText(const QString &text, HelpView::SearchDirection direction)
{
    QWebEnginePage::FindFlags flags = (ui->caseSenstivity->isChecked() ? QWebEnginePage::FindCaseSensitively : QWebEnginePage::FindFlags());
    if (direction == Backward)
        flags = flags | QWebEnginePage::FindBackward;
    ui->webEngineView->page()->findText(text, flags, [this](bool found) {
        if (found)
            ui->statusText->setText("");
        else
            ui->statusText->setText("No occurrences found");
    });
    if (text.isEmpty())
        ui->statusText->setText("");
}

} // namespace studio
} // namespace gams
