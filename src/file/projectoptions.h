#ifndef GAMS_STUDIO_PROJECTOPTIONS_H
#define GAMS_STUDIO_PROJECTOPTIONS_H

#include <QDialog>
#include <QLineEdit>

namespace gams {
namespace studio {
class PExProjectNode;

namespace project {

namespace Ui {
class ProjectOptions;
}


class ProjectOptions : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectOptions(QWidget *parent = nullptr);
    ~ProjectOptions() override;

    void showProject(PExProjectNode *project);

public slots:
    void accept() override;

private slots:
    void on_edWorkDir_textEdited(const QString &text);
    void on_edBaseDir_textEdited(const QString &text);
    void on_bWorkDir_clicked();
    void on_bBaseDir_clicked();

private:
    void updateEditColor(QLineEdit *edit, const QString &text);
    void showDirDialog(const QString &title, QLineEdit *lineEdit);

    Ui::ProjectOptions *ui;
    PExProjectNode *mProject = nullptr;

};


} // namespace project
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_PROJECTOPTIONS_H
