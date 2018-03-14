#ifndef GAMS_STUDIO_LXIVIEWER_LXIVIEWER_H
#define GAMS_STUDIO_LXIVIEWER_LXIVIEWER_H

#include "editors/codeeditor.h"
#include <QWidget>

namespace gams {
namespace studio {
namespace lxiviewer {

namespace Ui {
class LxiViewer;
}

class LxiViewer : public QWidget
{
    Q_OBJECT

public:
    explicit LxiViewer(CodeEditor *codeEditor, QString lstFile, QWidget *parent);
    ~LxiViewer();

private:
    Ui::LxiViewer *ui;

    CodeEditor* mCodeEditor;
    QString mLstFile;
    QString mLxiFile;
};


} // namespace lxiviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_LXIVIEWER_LXIVIEWER_H
