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
#include "welcomepage.h"
#include "commonpaths.h"
#include "settings.h"
#include "ui_welcomepage.h"
#include "mainwindow.h"
#include "wplabel.h"
#include "theme.h"
#include "file/fileicon.h"

#ifdef QWEBENGINE
#include <QWebEngineHistory>
#include <QWebEngineScriptCollection>
#include "overview.h"
#endif

namespace gams {
namespace studio {

static QString COverviewFile("RN_%1.html");

WelcomePage::WelcomePage(MainWindow *parent)
    : AbstractView(parent)
    , ui(new Ui::WelcomePage)
    , mMain(parent)
{
    ui->setupUi(this);
    if (!Settings::settings()->toBool(skSupressWebEngine))
        initReleaseOverview();
    checkReleaseNotes();
    ui->textBrowserChangelog->document()->setIndentWidth(16);

    connect(this, &WelcomePage::relayActionWp, mMain, [this](const QString &action) {
        if (action == "whatsNew") {
            showReleaseOverview();
        } else if (action == "openChangelog") {
            QString path;
            if (getChangelogPath(path) != fsMiss) {
                ui->textBrowserChangelog->setSource(QUrl::fromLocalFile(path), QTextDocument::MarkdownResource);
                ui->stackedWidget->setCurrentIndex(2);
            }
        }
    });

    historyChanged();
    mOutputVisible = mMain->outputViewVisibility();

    QString path = CommonPaths::documentationDir() + "/";
    QString docs = ui->label_doc_studio->property("documentation").toString();
    ui->label_doc_studio->setProperty("documentation", path + docs);
    docs = ui->label_doc_tut->property("documentation").toString();
    ui->label_doc_tut->setProperty("documentation", path + docs);
    if (!canShowReleaseOverview())
        ui->label_whats_new->setEnabled(false);
    QString clPath;
    FileState fState = getChangelogPath(clPath);
    if (fState == fsMiss) {
        QTimer::singleShot(0, this, [this]() {
            mMain->appendSystemLogWarning("Studio changelog not found.");
        });
        ui->label_changelog->setEnabled(false);
    } else if (fState == fsOther) {
        QTimer::singleShot(0, this, [this, clPath]() {
            mMain->appendSystemLogWarning("Studio changelog from GAMS installation: '"
                                          + QDir::toNativeSeparators(clPath) + "'");
        });
    }


    setupIcons();
    auto p = palette();
    p.setColor(QPalette::Window, p.color(QPalette::Base).lighter());

    connect(this, &WelcomePage::relayActionWp, parent, &MainWindow::receiveAction);
    connect(this, &WelcomePage::relayModLibLoad, parent, &MainWindow::receiveModLibLoad);
    connect(this, &WelcomePage::relayDocOpen, parent, &MainWindow::receiveOpenDoc);
    connect(this, &WelcomePage::openHelp, parent, &MainWindow::openHelp);
    connect(this, &WelcomePage::zoomRequest, this, &WelcomePage::handleZoom);
}

void WelcomePage::initReleaseOverview()
{
    ui->stackedWidget->setCurrentIndex(0);
    bool replaced = false;
    if (docPath(replaced).isEmpty()) return;
    if (replaced) return;
#ifdef QWEBENGINE
    QLayout *vLayout = ui->pageOverview->layout();

    // add Overview page
    mOverview = new Overview(this);
    mOverview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ui->pageOverview->setContentsMargins(0,0,0,0);
    ui->pageOverview->setLayout(vLayout);

    connect(mOverview, &QWebEngineView::urlChanged, this, [this](const QUrl &url) {
        bool replaced = false;
        QString path = docPath(replaced);
        if (replaced || (url.path() != path && url.path() != '/'+path)) {
            emit openHelp(url);
            if (mOverview->history()->canGoBack())
                mOverview->back();
        }
        ui->bBack->setEnabled(mOverview->history()->canGoBack());
    });
    connect(ui->bClose, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentIndex(0);
    });
    connect(ui->bCloseChangelog, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentIndex(0);
    });
    connect(ui->bBack, &QPushButton::clicked, this, [this]() {
        if (mOverview->history()->canGoBack()) {
            mOverview->history()->back();
            ui->bBack->setEnabled(mOverview->history()->canGoBack());
        }
    });

    connect(mOverview, &QWebEngineView::loadFinished, this, [this]() {
        setDarkMode(Theme::instance()->isDark());
    });
    connect(Theme::instance(), &Theme::changed, this, [this]() {
        setDarkMode(Theme::instance()->isDark());
    });

    vLayout->addWidget(mOverview);
#endif

#ifdef __APPLE__
    QLayoutItem *item = ui->buttonsLayout->takeAt(1);
    ui->buttonsLayout->insertItem(0, item);
    item = ui->buttonsLayout->takeAt(2);
    ui->buttonsLayout->insertItem(0, item);
    item = ui->buttonsLayout2->takeAt(1);
    ui->buttonsLayout2->insertItem(0, item);
#endif
}

QString WelcomePage::docPath(bool &replaced)
{
    QString path = getenv("STUDIO_RELEASE_OVERVIEW_PATH");
    if (!path.isEmpty()) {
        path = QDir::fromNativeSeparators(path);
        if (!path.endsWith("/"))
            path += "/";
        path += COverviewFile.arg(GAMS_DISTRIB_MAJOR);
        if (!QFile::exists(path))
            path.clear();
        else
            DEB() << "Redirected using STUDIO_RELEASE_OVERVIEW_PATH to: " << path;
    }
    if (path.isEmpty() && !CommonPaths::systemDir().isEmpty())
        path = CommonPaths::systemDir() + "/docs/";
    QString res;
    if (!path.isEmpty()) {
        replaced = false;
        res = path + COverviewFile.arg(GAMS_DISTRIB_MAJOR);
        if (!QFile::exists(res)) {
            replaced = true;
            res = path + COverviewFile.arg("MAIN");
            if (!QFile::exists(res))
                res = QString();
        }
    }
    return res;
}

FileState WelcomePage::getChangelogPath(QString &path)
{

    path = CommonPaths::changelog();
    if (QFile::exists(path))
        return fsExist;
    path = qApp->applicationDirPath() + "/resources/Changelog";
    if (QFile::exists(path))
        return fsExist;
#ifdef _WIN64
    path = CommonPaths::systemDir() + "/studio/resources/Changelog";
    if (QFile::exists(path))
        return fsOther;
#endif
    path = QString();
    return fsMiss;
}

FileState WelcomePage::checkReleaseNotes()
{
    bool replaced = false;
    QString path = docPath(replaced);
    if (path.isEmpty()) {
        if (!mMissRnWarned)
            QTimer::singleShot(0, this, [this]() {
                mMain->appendSystemLogWarning("Release notes not found in the GAMS installation at '" +
                                              CommonPaths::systemDir() + "/docs/'");
            });
        mMissRnWarned = true;
        return fsMiss;
    }
    if (replaced) {
        if (!mMissRnWarned)
            QTimer::singleShot(0, this, [this]() {
                mMain->appendSystemLogWarning(QString("Release notes for GAMS %1 not found.").arg(GAMS_DISTRIB_MAJOR) +
                                              " Showing main release note from '" + CommonPaths::systemDir() + "/docs/'");
            });
        mMissRnWarned = true;
        return fsOther;
    }
    return fsExist;
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
#ifdef QWEBENGINE
    if (mOverview)
        mOverview->page()->setZoomFactor(1);
#endif
}

void WelcomePage::setDocEnabled(bool enabled)
{
    ui->label_doc_studio->setEnabled(enabled);
    ui->label_doc_tut->setEnabled(enabled);
    ui->label_changelog->setEnabled(enabled);
    ui->label_whats_new->setEnabled(enabled && canShowReleaseOverview());
}

bool WelcomePage::canShowReleaseOverview()
{
    bool replaced = false;
    return !docPath(replaced).isEmpty();
}

bool WelcomePage::showReleaseOverview()
{
    bool replaced = false;
    QString path = docPath(replaced);
    if (path.isEmpty()) {
        return false;
    }
    if (replaced) {
        QUrl url = QUrl::fromLocalFile(path);
        emit openHelp(url);
        return false;
    }
    if (!mOverview) return false;
#ifdef QWEBENGINE
    mHideTOC = true;
    QUrl url = QUrl::fromLocalFile(path);
    url.setQuery("print=2");
    mOverview->load(url);
    ui->stackedWidget->setCurrentIndex(1);
#endif
    return true;
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
#ifdef QWEBENGINE
    if (mOverview && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_F5) {
            mOverview->reload();
        }
    }
#endif
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
#ifdef QWEBENGINE
    if (mOverview)
        mOverview->page()->setZoomFactor(mOverview->page()->zoomFactor() + (qreal(delta) / 10));
#endif
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

    ui->label_whats_new->setIndent(30);
    ui->label_whats_new->setIconSize(size);
    ui->label_whats_new->setIcon(Theme::icon(":/%1/new"));
    ui->label_changelog->setIndent(30);
    ui->label_changelog->setIconSize(size);
    ui->label_changelog->setIcon(Theme::icon(":/%1/scroll"));
    ui->label_gamsworld->setIndent(30);
    ui->label_gamsworld->setIconSize(size);
    ui->label_gamsworld->setIcon(Theme::icon(":/img/gams-w"));
    ui->label_contact->setIndent(30);
    ui->label_contact->setIconSize(size);
    ui->label_contact->setIcon(Theme::icon(":/%1/envelope"));
    ui->bBack->setIcon(Theme::icon(":/%1/backward"));
    ui->bClose->setIcon(Theme::icon(":/%1/remove"));
    ui->bCloseChangelog->setIcon(Theme::icon(":/%1/remove"));
}

void WelcomePage::setDarkMode(bool dark)
{
    if (!mOverview) return;
#ifdef QWEBENGINE
    if (!mOverview->page()->isLoading() && !mOverview->page()->url().path().isEmpty()) {
        mOverview->page()->runJavaScript(QString("DoxygenAwesomeDarkModeToggle.enableDarkMode(%1)")
                                             .arg(dark ? "true" : "false"));
        mIsDarkMode = dark;
    }
#endif
}


}
}
