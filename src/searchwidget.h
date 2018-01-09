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
    explicit SearchWidget(StudioSettings *settings, RecentData &rec, FileRepository &repo, QWidget *parent = 0);
    void find(bool backwards = false);
    ~SearchWidget();

    bool regex();
    bool caseSens();
    bool wholeWords();

private slots:
    void on_btn_Find_clicked();
    void on_btn_FindAll_clicked();
    void on_btn_Replace_clicked();
    void on_btn_ReplaceAll_clicked();
    void on_txt_search_returnPressed();

private:
    Ui::SearchWidget *ui;
    RecentData &mRecent;
    FileRepository &mRepo;
    QTextCursor mSelection;       // selected with find
    QTextCursor mLastSelection;   // last selection, as starting point for find next
    QList<TextMark*> mAllTextMarks;
    StudioSettings *mSettings;

    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *event);
    QFlags<QTextDocument::FindFlag> getFlags();
    void closeEvent(QCloseEvent *event);
};

}
}
#endif // SEARCHWIDGET_H
