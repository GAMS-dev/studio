#ifndef GAMS_STUDIO_PROJECTOPTIONS_H
#define GAMS_STUDIO_PROJECTOPTIONS_H

#include <QDialog>

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
    void on_toolButton_clicked();

private:
    Ui::ProjectOptions *ui;
    PExProjectNode *mProject = nullptr;

};


} // namespace project
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_PROJECTOPTIONS_H
