#ifndef GAMS_STUDIO_PROJECTOPTIONS_H
#define GAMS_STUDIO_PROJECTOPTIONS_H

#include <QFrame>
#include <QLineEdit>
#include "common.h"

namespace gams {
namespace studio {
class PExProjectNode;

namespace project {

namespace Ui {
class ProjectOptions;
}


class ProjectOptions : public QFrame
{
    Q_OBJECT

public:
    explicit ProjectOptions(QWidget *parent = nullptr);
    ~ProjectOptions() override;
    void setProject(PExProjectNode *project);
    bool isModified() const;
    void save();

signals:
    void modificationChanged(bool modification);

private slots:
    void on_edName_textChanged(const QString &text);
    void on_edWorkDir_textChanged(const QString &text);
    void on_edBaseDir_textChanged(const QString &text);
    void on_bWorkDir_clicked();
    void on_bBaseDir_clicked();
    void projectChanged(gams::studio::NodeId id);

private:
    void updateEditColor(QLineEdit *edit, const QString &text);
    void updateState();
    void showDirDialog(const QString &title, QLineEdit *lineEdit, QString defaultDir);

    Ui::ProjectOptions *ui;
    PExProjectNode *mProject = nullptr;
    bool mModified = false;
    QString mName;

};


} // namespace project
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_PROJECTOPTIONS_H
