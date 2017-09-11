/*
 * This file is part of the GAMS IDE project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMSIDE_H
#define GAMSIDE_H

#include <QMainWindow>
#include <QTextStream>
#include <QProcess>
#include <QMutex>

namespace Ui {
class GAMSIDE;
}

class GAMSIDE : public QMainWindow
{
    Q_OBJECT

public:
    explicit GAMSIDE(QWidget *parent = 0);
    ~GAMSIDE();

signals:
    void processOutput(QString text);

private slots:
    void clearProc(int exitCode);
    void addLine(QProcess::ProcessChannel channel, QString text);
    void readyStdOut();
    void readyStdErr();

private slots:
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionSave_All_triggered();
    void on_actionClose_triggered();
    void on_actionClose_All_triggered();
    void on_actionExit_Application_triggered();
    void on_actionOnline_Help_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionProject_Explorer_triggered(bool checked);
    void on_actionLog_Output_triggered(bool checked);
    void on_actionBottom_Panel_triggered(bool checked);
    void on_actionSim_Process_triggered();

private:
    Ui::GAMSIDE *ui;
    QProcess *mProc = nullptr;
    QHash<QTextStream, QColor> mStreams;
    QMutex mOutputMutex;

};

#endif // GAMSIDE_H
