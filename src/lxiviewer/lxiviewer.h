#ifndef GAMS_STUDIO_LXIVIEWER_LXIVIEWER_H
#define GAMS_STUDIO_LXIVIEWER_LXIVIEWER_H

#include <QWidget>
#include <QModelIndex>

namespace gams {
namespace studio {

class CodeEditor;
class FileContext;

namespace lxiviewer {

namespace Ui {
class LxiViewer;
}

class LxiViewer : public QWidget
{
    Q_OBJECT

public:
    explicit LxiViewer(CodeEditor *codeEditor, FileContext *fc, QWidget *parent);
    ~LxiViewer();

    CodeEditor *codeEditor() const;

private:
    Ui::LxiViewer *ui;

    CodeEditor* mCodeEditor;
    FileContext *mFileContext;
    QString mLstFile;
    QString mLxiFile;

private slots:
    void loadLxiFile();
    void jumpToTreeItem();


private slots:
    void jumpToLine(QModelIndex modelIndex);
};


} // namespace lxiviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_LXIVIEWER_LXIVIEWER_H
