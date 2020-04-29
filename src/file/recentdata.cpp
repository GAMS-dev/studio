#include "recentdata.h"
#include "mainwindow.h"
#include "settings.h"
#include "viewhelper.h"

namespace gams {
namespace studio {

void RecentData::setEditor(QWidget *edit, MainWindow* window)
{
    if (AbstractEdit* aEdit = ViewHelper::toAbstractEdit(mEditor)) {
        MainWindow::disconnect(aEdit, &AbstractEdit::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(aEdit, &AbstractEdit::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(aEdit, &AbstractEdit::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
        MainWindow::disconnect(aEdit->document(), &QTextDocument::contentsChange, window, &MainWindow::currentDocumentChanged);
    } else if (option::SolverOptionWidget* soEdit = ViewHelper::toSolverOptionEdit(mEditor)) {
        MainWindow::disconnect(soEdit, &option::SolverOptionWidget::itemCountChanged, window, &MainWindow::updateEditorItemCount );
    }
    if (TextView* tv = ViewHelper::toTextView(mEditor)) {
//        MainWindow::disconnect(tv, &TextView::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(tv, &TextView::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(tv, &TextView::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
        MainWindow::disconnect(tv, &TextView::loadAmountChanged, window, &MainWindow::updateLoadAmount);
        window->resetLoadAmount();
    }

    mEditor = edit;
    mGroup = window->projectRepo()->asGroup(ViewHelper::groupId(edit));
    if (ProjectFileNode* node = window->projectRepo()->findFileNode(edit)) {
        mEditFileId = node->file()->id();
        mPath = QFileInfo(node->location()).path();
    } else {
        mEditFileId = FileId();
        mPath = Settings::settings()->toString(skDefaultWorkspace);
    }

    if (AbstractEdit* aEdit = ViewHelper::toAbstractEdit(mEditor)) {
        MainWindow::connect(aEdit, &AbstractEdit::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(aEdit, &AbstractEdit::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(aEdit, &AbstractEdit::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
        MainWindow::connect(aEdit->document(), &QTextDocument::contentsChange, window, &MainWindow::currentDocumentChanged);
    } else if (option::SolverOptionWidget* soEdit = ViewHelper::toSolverOptionEdit(mEditor)) {
        MainWindow::connect(soEdit, &option::SolverOptionWidget::itemCountChanged, window, &MainWindow::updateEditorItemCount );
    }
    if (TextView* tv = ViewHelper::toTextView(mEditor)) {
        MainWindow::connect(tv, &TextView::selectionChanged, window, &MainWindow::updateEditorPos, Qt::UniqueConnection);
//        MainWindow::connect(tv, &TextView::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(tv, &TextView::blockCountChanged, window, &MainWindow::updateEditorBlockCount, Qt::UniqueConnection);
        MainWindow::connect(tv, &TextView::loadAmountChanged, window, &MainWindow::updateLoadAmount, Qt::UniqueConnection);
    }
    window->updateEditorMode();
    window->updateEditorPos();
}

void RecentData::reset()
{
    mEditFileId = -1;
    mPath = Settings::settings()->toString(skDefaultWorkspace);
    mGroup = nullptr;
    mEditor = nullptr;
}

bool RecentData::hasValidRunGroup()
{
    if (!group())
        return false;
    return group()->toRunGroup() != nullptr;
}


} // namespace studio
} // namespace gams
