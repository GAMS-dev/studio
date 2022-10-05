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
 */
#ifndef GAMS_STUDIO_NAVIGATORDIALOG_H
#define GAMS_STUDIO_NAVIGATORDIALOG_H

#include <QDialog>
#include "mainwindow.h"
#include "navigator/navigatormodel.h"

namespace Ui {
class NavigatorDialog;
}

namespace gams {
namespace studio {

enum class NavigatorMode { AllFiles, Line, Help, InProject, Tabs, Logs, FileSystem };

class NavigatorDialog : public QDialog
{
    Q_OBJECT

public:
    NavigatorDialog(MainWindow* main, NavigatorLineEdit *inputField);
    ~NavigatorDialog();
    void updatePosition();

public slots:
    void receiveKeyEvent(QKeyEvent *event);

private:
    void keyPressEvent(QKeyEvent* e) override;
    void showEvent(QShowEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void setInput(const QString& input);

    void collectHelpContent(QVector<NavigatorContent> &content);
    void collectLineNavigation(QVector<NavigatorContent> &content);
    void collectAllFiles(QVector<NavigatorContent> &content);
    void collectOpenFiles(QVector<NavigatorContent> &content);
    void collectInProject(QVector<NavigatorContent> &content);
    void collectTabs(QVector<NavigatorContent> &content);
    void collectLogs(QVector<NavigatorContent> &content);
    void collectFileSystem(QVector<NavigatorContent> &content);
    bool valueExists(FileMeta *fm, const QVector<NavigatorContent>& content);
    void updateContent(NavigatorMode mode);
    void selectFileOrFolder(NavigatorContent nc);
    void selectHelpContent(NavigatorContent nc);
    void selectLineNavigation();

    ///
    /// \brief findClosestPath removes characters from the current string
    /// until it finds a valid directory
    /// \param path
    /// \return
    ///
    QDir findClosestPath(const QString& path);

private slots:
    void returnPressed();

private:
    Ui::NavigatorDialog* ui = nullptr;
    MainWindow* mMain = nullptr;
    NavigatorModel* mNavModel = nullptr;
    NavigatorLineEdit* mInput = nullptr;
    QSortFilterProxyModel* mFilterModel = nullptr;
    NavigatorMode mCurrentMode = NavigatorMode::AllFiles;
    bool mDirSelectionOngoing = false;
    QDir mSelectedDirectory;
};

}
}
#endif // GAMS_STUDIO_NAVIGATORDIALOG_H
