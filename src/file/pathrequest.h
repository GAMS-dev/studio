#ifndef GAMS_STUDIO_PATH_PATHREQUEST_H
#define GAMS_STUDIO_PATH_PATHREQUEST_H

#include <QDialog>
#include <QLineEdit>

namespace gams {
namespace studio {

class ProjectRepo;

namespace path {

namespace Ui {
class PathRequest;
}

class PathRequest : public QDialog
{
    Q_OBJECT

public:
    explicit PathRequest(QWidget *parent = nullptr);
    ~PathRequest() override;
    void init(ProjectRepo *repo, const QString &baseDir, const QVariantList &data);
    bool checkProject();
    QString baseDir() const;

private slots:
    void on_bDir_clicked();

    void on_bCheck_clicked();

    void on_edBaseDir_textChanged(const QString &text);

private:
    void showDirDialog(const QString &title, QLineEdit *lineEdit);

private:
    Ui::PathRequest *ui;
    ProjectRepo *mProjectRepo = nullptr;
    QVariantList mData;
    QString mInitialText;
    QString mInitalBasePath;

};


} // namespace path
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_PATH_PATHREQUEST_H
