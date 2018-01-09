#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "mainwindow.h"
#include <QDialog>

namespace Ui {
class SearchWidget;
}

namespace gams {
namespace studio {

class SearchWidget : public QDialog
{
    Q_OBJECT

public:
    explicit SearchWidget(StudioSettings *settings, RecentData &rec, FileRepository &repo, MainWindow *parent = 0);
    void find(bool backwards = false);
    ~SearchWidget();

    bool regex();
    bool caseSens();
    bool wholeWords();
    QString searchTerm();

    int selectedScope();
    void setSelectedScope(int index);
private slots:
    void on_btn_Find_clicked();
    void on_btn_FindAll_clicked();
    void on_btn_Replace_clicked();
    void on_btn_ReplaceAll_clicked();
    void on_txt_search_returnPressed();

    void on_btn_close_clicked();

private:
    Ui::SearchWidget *ui;
    StudioSettings *mSettings;
    RecentData &mRecent;
    FileRepository &mRepo;
    MainWindow *mMain;
    QTextCursor mSelection;       // selected with find
    QTextCursor mLastSelection;   // last selection, as starting point for find next
    QList<TextMark*> mAllTextMarks;

    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *event);
    QFlags<QTextDocument::FindFlag> getFlags();
    void closeEvent(QCloseEvent *event);
    void simpleFindAndHighlight(QPlainTextEdit *edit = nullptr);
    QList<Result> findInGroup(FileSystemContext *fsc = nullptr);
    QList<Result> findInFile(FileSystemContext *fsc);
};

class Result
{
private:
    int mLocLineNr;
    QString mLocFile;
    QString mContext;

public:
    explicit Result(int locLineNr, QString locFile, QString context = "");
    int locLineNr() const;
    QString locFile() const;
    QString context() const;
};

}
}
#endif // SEARCHWIDGET_H
