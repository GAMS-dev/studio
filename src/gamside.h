#ifndef GAMSIDE_H
#define GAMSIDE_H

#include <QMainWindow>
#include <QTextStream>
#include <QProcess>
#include <QMutex>
#include "filerepository.h"
#include "tabwidget.h"
#include "treemodel.h"

namespace Ui {
class GAMSIDE;
}

using namespace gams::ide;

class GAMSIDE : public QMainWindow
{
    Q_OBJECT
public:
    explicit GAMSIDE(QWidget *parent = 0);
    ~GAMSIDE();
    void createEdit(gams::ide::TabWidget* tabWidget, QString codecName = QString());
    void createEdit(gams::ide::TabWidget* tabWidget, int id = -1, QString codecName = QString());
    void createTreeEntry(const FileContext* fc);
    void ensureCodecMenue(QString codecName);

signals:
    void processOutput(QString text);

private slots:
    void clearProc(int exitCode);
    void addLine(QProcess::ProcessChannel channel, QString text);
    void readyStdOut();
    void readyStdErr();
    void codecChanged(QAction *action);

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
    FileRepository mFileRepo;
    QMutex mOutputMutex;
    QActionGroup *mCodecGroup;
    TreeModel mProjectModel;
};

#endif // GAMSIDE_H
