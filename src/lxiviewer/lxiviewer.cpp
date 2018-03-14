#include "lxiviewer.h"
#include "lxiparser.h"
#include "lxitreemodel.h"
#include "ui_lxiviewer.h"

namespace gams {
namespace studio {
namespace lxiviewer {

LxiViewer::LxiViewer(CodeEditor *codeEditor, QString lstFile, QWidget *parent):
    QWidget(parent), mCodeEditor(codeEditor), mLstFile(lstFile),
    ui(new Ui::LxiViewer)
{
    ui->setupUi(this);

    ui->splitter->addWidget(mCodeEditor);

    QFileInfo info(mLstFile);
    mLxiFile = info.path() + "/" + info.baseName() + ".lxi";

    if (QFileInfo(mLxiFile).exists()) {
        LxiTreeModel* model = new LxiTreeModel(LxiParser::parseFile(QDir::toNativeSeparators(mLxiFile)));
        ui->lxiTreeView->setModel(model);
    }
}

LxiViewer::~LxiViewer()
{
    delete ui;
}

} // namespace lxiviewer
} // namespace studio
} // namespace gams
