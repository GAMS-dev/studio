/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include <QKeyEvent>
#include <QStatusBar>
#include <QApplication>
#include <QAbstractItemModel>

#include "ui_navigatordialog.h"
#include "navigator/navigatorlineedit.h"
#include "navigatordialog.h"

namespace gams {
namespace studio {

NavigatorDialog::NavigatorDialog(MainWindow *main, NavigatorLineEdit* inputField)
    : QDialog((QWidget*)main), ui(new Ui::NavigatorDialog), mMain(main), mInput(inputField)
{
    setWindowFlags(Qt::ToolTip);

    ui->setupUi(this);
    mNavModel = new NavigatorModel(this);
    mFilterModel = new QSortFilterProxyModel(this);

    mFilterModel->setSourceModel(mNavModel);
    mFilterModel->sort(0);
    mFilterModel->setDynamicSortFilter(false);
    mFilterModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mFilterModel->setFilterKeyColumn(0);

    ui->tableView->setModel(mFilterModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->installEventFilter(this);

    connect(mInput, &QLineEdit::returnPressed, this, &NavigatorDialog::returnPressed);
    connect(ui->tableView, &QTableView::clicked, this, &NavigatorDialog::itemClicked);
    connect(mInput, &QLineEdit::textEdited, this, &NavigatorDialog::inputChanged);
    connect(mInput, &NavigatorLineEdit::autocompleteTriggered, this, &NavigatorDialog::autocomplete);
    connect(mInput, &FilterLineEdit::regExpChanged, this, &NavigatorDialog::regexChanged);
}

NavigatorDialog::~NavigatorDialog()
{
    delete ui;
    delete mNavModel;
    delete mFilterModel;
}

void NavigatorDialog::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)

    updatePosition();
    inputChanged();
}

void NavigatorDialog::highlightCurrentFile()
{
    if (!mFilterModel) return;

    if (FileMeta* fm = mMain->fileRepo()->fileMeta(mMain->recent()->editor())) {
        int index = mNavModel->findIndex(fm->location());
        if (index >= 0) {
            QModelIndex filterModelIndex = mFilterModel->mapFromSource(mNavModel->index(index, 0));

            if (filterModelIndex.isValid()) {
                ui->tableView->setCurrentIndex(filterModelIndex);
                ui->tableView->scrollTo(filterModelIndex);
                return;
            }
        }
    }
    ui->tableView->setCurrentIndex(mFilterModel->index(0, 0));
    ui->tableView->scrollTo(mFilterModel->index(0, 0));
}

void NavigatorDialog::inputChanged()
{
    if (!isVisible())
        show();

    updateContent();
}

void NavigatorDialog::changeEvent(QEvent*)
{
    conditionallyClose();
}

void NavigatorDialog::updateContent()
{
    QVector<NavigatorContent> content = QVector<NavigatorContent>();
    QString input = mInput->text();

    QRegularExpressionMatch match;
    if (input.startsWith("?")) {
        mCurrentMode = NavigatorMode::Help;
        setFilter("", true);
        collectHelpContent(content);

    } else if (mPostfixRegex.match(input).hasMatch()) {
        mCurrentMode = NavigatorMode::Line;
        setFilter("", true);
        collectLineNavigation(content);

    } else if (input.startsWith("p ", Qt::CaseInsensitive)) {
        mCurrentMode = NavigatorMode::InProject;
        setFilter(input.remove(mPrefixRegex));
        collectInProject(content);

    } else if (input.startsWith("t ", Qt::CaseInsensitive)) {
        mCurrentMode = NavigatorMode::Tabs;
        setFilter(input.remove(mPrefixRegex));
        collectTabs(content);

    } else if (input.startsWith("l ", Qt::CaseInsensitive)) {
        mCurrentMode = NavigatorMode::Logs;
        setFilter(input.remove(mPrefixRegex));
        collectLogs(content);

    } else if (input.startsWith("f ", Qt::CaseInsensitive)) {
        mCurrentMode = NavigatorMode::FileSystem;
        collectFileSystem(content);

    } else if (input.startsWith("x ", Qt::CaseInsensitive)) {
        mCurrentMode = NavigatorMode::QuickAction;
        mFilterModel->setFilterWildcard(input.remove(mPrefixRegex));
        collectQuickActions(content);

    } else {
        setFilter(input);
        mFilterModel->setFilterWildcard(input);
        collectAllFiles(content);
    }

    mNavModel->setContent(content);
    if (mCurrentMode != NavigatorMode::FileSystem) {
        mNavModel->setCurrentDir(QDir(mMain->recent()->path()));
        mDirSelectionOngoing = false;
    }

    if (!ui->tableView->currentIndex().isValid())
        highlightCurrentFile();
}

void NavigatorDialog::collectHelpContent(QVector<NavigatorContent> &content)
{
    content.append(NavigatorContent(":number", "jump to line number", ":",
                                    mMain->fileRepo()->fileMeta(mMain->recent()->editor())));
    content.append(NavigatorContent("filename", "filter all files", ""));
    content.append(NavigatorContent("P filename", "filter files in current project", "P "));
    content.append(NavigatorContent("T filename", "filter open tabs", "T "));
    content.append(NavigatorContent("L filename", "filter logs", "L "));
    content.append(NavigatorContent("F filename", "files in filesystem", "F "));
    content.append(NavigatorContent("X quick action", "access often used Studio functions", "X "));
}

void NavigatorDialog::collectAllFiles(QVector<NavigatorContent> &content)
{
    collectTabs(content);
    collectInProject(content);
    collectLogs(content);

    foreach (FileMeta* fm, mMain->fileRepo()->fileMetas()) {
        if (!valueExists(fm, content) && !fm->location().endsWith("~log")) {
            content.append(NavigatorContent(fm, "Known Files"));
        }
    }
}

void NavigatorDialog::collectInProject(QVector<NavigatorContent> &content)
{
    PExFileNode* currentFile = mMain->projectRepo()->findFileNode(mMain->currentEdit());
    if (!currentFile) return;

    for (PExFileNode* f : currentFile->assignedProject()->listFiles()) {
        FileMeta* fm = f->file();
        if (!valueExists(fm, content)) {
            content.append(NavigatorContent(fm, "Current Project"));
        }
    }
}

void NavigatorDialog::collectTabs(QVector<NavigatorContent> &content)
{
    foreach (FileMeta* fm, mMain->fileRepo()->openFiles()) {
        if (!valueExists(fm, content) && !fm->location().endsWith("~log")) {
            content.append(NavigatorContent(fm, "Open Tabs"));
        }
    }
}

void NavigatorDialog::collectLogs(QVector<NavigatorContent> &content)
{
    foreach (PExProjectNode* project, mMain->projectRepo()->projects()) {
        PExLogNode* log = project->logNode();

        FileMeta* fm = log->file();
        if (fm->editors().empty()) continue;

        content.append(NavigatorContent(fm, "Open Logs"));
    }
}

void NavigatorDialog::collectFileSystem(QVector<NavigatorContent> &content)
{
    QString textInput = mInput->text();
    textInput.remove(mPrefixRegex);

    QDir dir(textInput);
    if (!mDirSelectionOngoing) {
        mSelectedDirectory = mNavModel->currentDir();
        mDirSelectionOngoing = true;
    } else if (dir.exists()) {
        if (dir.isRelative()) {
            mSelectedDirectory.setPath(mNavModel->currentDir().absolutePath() + QDir::separator() + textInput);
        } else mSelectedDirectory = dir;

        textInput.append(QDir::separator()); // we need paths to end in seperator for better usability
    } else {
        mSelectedDirectory = findClosestPath(textInput);
    }

    // filter prefix and extract relevant wildcard term
    QString filter = textInput.right(textInput.length() - textInput.lastIndexOf(QDir::separator()));
    filter.remove(QDir::separator());
    setFilter(filter);

    QFileInfoList localEntryInfoList = mSelectedDirectory.entryInfoList(
                                           QDir::NoDot | QDir::AllEntries,
                                           QDir::Name | QDir::DirsFirst);
    for (const QFileInfo &entry : qAsConst(localEntryInfoList)) {
        content.append(NavigatorContent(entry, entry.isDir() ? "Directory" : "File"));
    }
}

void NavigatorDialog::collectLineNavigation(QVector<NavigatorContent> &content)
{
    FileMeta* fm = nullptr;
    QFileInfo fi;
    QModelIndex index = ui->tableView->currentIndex();

    // chained file selection and line navigation
    NavigatorContent nc;
    if (index.isValid()) {
        QModelIndex mappedIndex = mFilterModel->mapToSource(index);
        nc = mNavModel->content().at(mappedIndex.row());

    } else if (mLastSelectedItem.isValid()) {
        nc = mLastSelectedItem;
    }
    fm = nc.GetFileMeta();
    fi = nc.FileInfo();

    if (fm) {
        // if has editors, get line number
        if (fm->editors().count()) {
            content.append(
                        NavigatorContent(fm, "Max Lines: " + QString::number(mMain->linesInEditor(
                                                                fm->editors().constFirst())))
                        );
        } else {  // unknown line number
            content.append(NavigatorContent(fm, "Max Lines: Unknown"));
        }

    } else {
        // unkown files have no fileMeta, so use QFileInfo instead
        content.append(NavigatorContent(fi, "Max Lines: Unknown"));
    }
}

void NavigatorDialog::collectQuickActions(QVector<NavigatorContent> &content)
{
    // dialogs
    content.append(NavigatorContent("Open Settings",
                                    std::bind(&MainWindow::on_actionSettings_triggered, mMain)));
    content.append(NavigatorContent("Open Model Library Explorer",
                                    std::bind(&MainWindow::on_actionGAMS_Library_triggered, mMain)));
    content.append(NavigatorContent("Open Terminal",
                                    std::bind(&MainWindow::on_actionTerminal_triggered, mMain)));
    content.append(NavigatorContent("GDX Diff",
                                    std::bind(&MainWindow::on_actionGDX_Diff_triggered, mMain)));

    // projects and files
    content.append(NavigatorContent("New Project",
                                    std::bind(&MainWindow::on_actionNew_Project_triggered, mMain)));
    content.append(NavigatorContent("Open...",
                                    std::bind(&MainWindow::on_actionOpen_triggered, mMain)));
    content.append(NavigatorContent("Open Folder...",
                                    std::bind(&MainWindow::on_actionOpen_Folder_triggered, mMain)));
    content.append(NavigatorContent("Save All",
                                    std::bind(&MainWindow::on_actionSave_All_triggered, mMain)));
    content.append(NavigatorContent("Close All",
                                    std::bind(&MainWindow::on_actionClose_All_triggered, mMain)));
    content.append(NavigatorContent("Clean scratch dirs",
                                    std::bind(&MainWindow::on_actionDeleteScratchDirs_triggered, mMain)));
    content.append(NavigatorContent("Edit Default GAMS Configuration",
                                    std::bind(&MainWindow::on_actionEditDefaultConfig_triggered, mMain)));

    // views
    content.append(NavigatorContent("Toggle Fullscreen",
                                    std::bind(&MainWindow::toggleFullscreen, mMain)));
    content.append(NavigatorContent("Toggle Distraction-free mode",
                                    std::bind(&MainWindow::toggleDistractionFreeMode, mMain)));

    // run
    content.append(NavigatorContent("Run GAMS",
                                    std::bind(&MainWindow::on_actionRun_triggered, mMain)));
    content.append(NavigatorContent("Run GAMS with GDX creation",
                                    std::bind(&MainWindow::on_actionRun_with_GDX_Creation_triggered, mMain)));
    content.append(NavigatorContent("Run NEOS",
                                    std::bind(&MainWindow::on_actionRunNeos_triggered, mMain)));
    content.append(NavigatorContent("Run GAMS Engine",
                                    std::bind(&MainWindow::on_actionRunEngine_triggered, mMain)));

    content.append(NavigatorContent("Run MIRO base mode",
                                    std::bind(&MainWindow::on_actionBase_mode_triggered, mMain)));
    content.append(NavigatorContent("Run MIRO configuration mode",
                                    std::bind(&MainWindow::on_actionConfiguration_mode_triggered, mMain)));
}

void NavigatorDialog::returnPressed()
{
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) return;

    selectItem(index);
}

void NavigatorDialog::selectItem(QModelIndex index)
{
    QModelIndex mappedIndex = mFilterModel->mapToSource(index);

    if (mCurrentMode == NavigatorMode::Line) {
        // if different file then current, change file first
        if (index.isValid() && !mInput->text().startsWith(":"))
            selectFileOrFolder(mNavModel->content().at(mappedIndex.row()));

        selectLineNavigation();
        return;
    }

    if (!mappedIndex.isValid()) return;
    NavigatorContent nc = mNavModel->content().at(mappedIndex.row());

    if (mCurrentMode == NavigatorMode::Help)
        selectHelpContent(nc);
    else if (mCurrentMode == NavigatorMode::QuickAction)
        selectQuickAction(nc);
    else selectFileOrFolder(nc);
}

void NavigatorDialog::autocomplete()
{
    QModelIndex mappedIndex = mFilterModel->mapToSource(ui->tableView->currentIndex());
    if (!mappedIndex.isValid() && !mLastSelectedItem.isValid()) return;

    NavigatorContent nc = mNavModel->content().at(mappedIndex.row());
    mLastSelectedItem = nc;

    QRegularExpressionMatch preMatch = mPrefixRegex.match(mInput->text());
    QRegularExpressionMatch postMatch = mPostfixRegex.match(mInput->text());
    QString prefix, postfix;

    if (preMatch.hasMatch())
        prefix = preMatch.captured(1) + " ";

    if (postMatch.hasMatch())
        postfix = postMatch.captured(1);

    if (!nc.Prefix().isEmpty()) { // help content
        FileMeta* fm = mMain->fileRepo()->fileMeta(mMain->recent()->editor());
        if (fm) mInput->setText(prefix + fm->location() + postfix);
    } else if (nc.GetFileMeta()) {
        mInput->setText(prefix + (nc.Text().isEmpty() ? nc.FileInfo().fileName() : nc.Text()) + postfix);
    } else {
        mInput->setText(prefix + nc.FileInfo().absoluteFilePath() + postfix);
    }
}

void NavigatorDialog::fillFileSystemPath(NavigatorContent nc)
{
    mSelectedDirectory = QDir(nc.FileInfo().absoluteFilePath());
    mInput->setText("f " + QDir::toNativeSeparators(mSelectedDirectory.absolutePath())
                         + (nc.FileInfo().isDir() ? QDir::separator() : QString()));
    updateContent();
}

void NavigatorDialog::selectFileOrFolder(NavigatorContent nc)
{
    if (FileMeta* fm = nc.GetFileMeta()) {
        if (fm->location().endsWith("~log"))
            mMain->jumpToTab(fm);
        else mMain->openFile(fm, true);

        close();
    } else {
        if (nc.FileInfo().isFile()) {
            mMain->openFilePath(nc.FileInfo().absoluteFilePath(), nullptr, OpenGroupOption::ogNone, true);
            close();
        } else {
            fillFileSystemPath(nc);
        }
    }
}

void NavigatorDialog::selectHelpContent(NavigatorContent nc)
{
    mInput->setText(nc.Prefix());
    updateContent();
}

void NavigatorDialog::selectQuickAction(NavigatorContent nc)
{
    nc.ExecuteQuickAction();
    mInput->setText("");
    close();
}

void NavigatorDialog::selectLineNavigation()
{
    QRegularExpressionMatch match = mPostfixRegex.match(mInput->text());
    if (!match.captured(1).isEmpty())
        mMain->jumpToLine(match.captured(1).remove(0,1).toInt()-1); // remove leading colon from match

    close();
}

bool NavigatorDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != ui->tableView) return false;
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        keyPressEvent(keyEvent);
        return true;
    }
    return false;
}

void NavigatorDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Down) {
        int pos = ui->tableView->currentIndex().row() + 1;
        if (pos >= ui->tableView->model()->rowCount())
            pos = 0;

        ui->tableView->setCurrentIndex(mFilterModel->index(pos, 0));
    } else if (e->key() == Qt::Key_Up) {
        int pos = ui->tableView->currentIndex().row() - 1;
        if (pos < 0)
            pos = ui->tableView->model()->rowCount() - 1;

        ui->tableView->setCurrentIndex(mFilterModel->index(pos, 0));
    } else if (e->key() == Qt::Key_Escape) {
        close();
    } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        returnPressed();
    } else {
        mInput->keyPressEvent(e);
    }
}

void NavigatorDialog::itemClicked(const QModelIndex &index)
{
    if (index.isValid())
        selectItem(index);
}

void NavigatorDialog::regexChanged(QRegExp regex)
{
    Q_UNUSED(regex)

    // clicking outside the dialog closes it, but we dont want that for changing the regex
    setVisible(true);
    mInput->setFocus(Qt::FocusReason::PopupFocusReason);
}

void NavigatorDialog::setFilter(QString filter, bool ignoreOptions)
{
    QString regex = filter;

    if (!ignoreOptions) {
        if (mInput->regExp().patternSyntax() != QRegExp::RegExp)
            regex = QRegularExpression::escape(filter);

        if (mInput->exactMatch()) regex = '^' + filter + '$';
    }

    mFilterModel->setFilterRegExp(regex);
}

void NavigatorDialog::updatePosition()
{
    if (!isVisible()) return;

    QPoint position;

    position.setX(qMin(mInput->pos().x(), mMain->width() - width()));
    position.setY(mMain->height() - height() - mMain->statusBar()->height());

    move(mMain->mapToGlobal(position));
}

void NavigatorDialog::receiveKeyEvent(QKeyEvent *event)
{
    keyPressEvent(event);
}

bool NavigatorDialog::conditionallyClose()
{
    if (QApplication::activeWindow() == this)
        return false;
    else return QDialog::close();
}

bool NavigatorDialog::valueExists(FileMeta* fm, const QVector<NavigatorContent>& content)
{
    foreach (NavigatorContent c, content) {
        if (c.GetFileMeta() == fm)
            return true;
    }
    return false;
}

QDir NavigatorDialog::findClosestPath(const QString& path)
{
    QString tryPath = path;

    int i = 10;
    while(tryPath.length() > 0) {
        tryPath = tryPath.left(tryPath.length()-1);
        if (tryPath.isEmpty()) tryPath = mNavModel->currentDir().absolutePath();

        QDir d(tryPath);
        if (d.exists()) {
            return d;
        }

        // infinite loop safeguard
        if (i-- == 0)
            break;
    }
    return QDir();
}

}
}
