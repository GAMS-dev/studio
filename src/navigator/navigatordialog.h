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
#ifndef GAMS_STUDIO_NAVIGATORDIALOG_H
#define GAMS_STUDIO_NAVIGATORDIALOG_H

#include <QDialog>
#include "mainwindow.h"
#include "navigator/navigatorcontent.h"
#include "navigator/navigatormodel.h"
#include "qregularexpression.h"

namespace Ui {
class NavigatorDialog;
}

namespace gams {
namespace studio {

enum class NavigatorMode { AllFiles, Line, Help, InProject, Tabs, Logs, FileSystem, QuickAction };

class NavigatorDialog : public QDialog
{
    Q_OBJECT

public:
    NavigatorDialog(MainWindow* main, NavigatorLineEdit *inputField);
    ~NavigatorDialog();
    void updatePosition();

public slots:
    void receiveKeyEvent(QKeyEvent *event);
    bool conditionallyClose();
    void activeFileChanged();

private:
    void keyPressEvent(QKeyEvent* e) override;
    void showEvent(QShowEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void inputChanged();
    void changeEvent(QEvent*) override;

    void collectHelpContent(QSet<NavigatorContent> &content);
    void handleLineNavigation(QVector<NavigatorContent> &content, int lineNr);
    void collectAllFiles(QSet<NavigatorContent> &content);
    void collectOpenFiles(QVector<NavigatorContent> &content);
    void collectInProject(QSet<NavigatorContent> &content);
    void collectTabs(QSet<NavigatorContent> &content);
    void collectLogs(QSet<NavigatorContent> &content);
    void collectFileSystem(QSet<NavigatorContent> &content);
    void collectLineNavigation(QSet<NavigatorContent> &content);
    void collectQuickActions(QSet<NavigatorContent> &content);
    void updateContent();
    void selectFileOrFolder(const NavigatorContent& nc);
    void selectHelpContent(const NavigatorContent& nc);
    void selectQuickAction(const NavigatorContent& nc);
    void selectLineNavigation();
    void selectItem(QModelIndex index);
    void autocomplete();
    void fillFileSystemPath(const NavigatorContent& nc);
    void highlightCurrentFile();
    void setFilter(const QString &filter);

    ///
    /// \brief findClosestPath removes characters from the current string
    /// until it finds a valid directory
    /// \param path
    /// \return
    ///
    QDir findClosestPath(const QString& path);

private slots:
    void returnPressed();
    void itemClicked(const QModelIndex& index);
    void regexChanged(const QRegularExpression &regex);

private:
    Ui::NavigatorDialog* ui = nullptr;
    MainWindow* mMain = nullptr;
    NavigatorModel* mNavModel = nullptr;
    NavigatorLineEdit* mInput = nullptr;
    QSortFilterProxyModel* mFilterModel = nullptr;
    NavigatorMode mCurrentMode = NavigatorMode::AllFiles;
    bool mDirSelectionOngoing = false;
    QDir mSelectedDirectory;
    QRegularExpression mPrefixRegex = QRegularExpression("^(\\w) "); // starts with prefix
    QRegularExpression mPostfixRegex = QRegularExpression("(:\\d+)$"); // has trailing line number
    QRegularExpression mWindowsPath = QRegularExpression("^\\w\\:\\\\");
    NavigatorContent mLastSelectedItem;
    bool mUseRegex = false;
    bool mWholeWords = false;
};

}
}
#endif // GAMS_STUDIO_NAVIGATORDIALOG_H
