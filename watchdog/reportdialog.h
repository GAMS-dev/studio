#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QDialog>
#include <QTemporaryDir>

namespace gams {
namespace studio {

namespace Ui {
class Dialog;
}

enum DialogKind {
    dkUnresponsive,
    dkCrash
};

enum TypedIndex {
    tiEmail,
    tiLog,
    tiLogBk,
    tiSettings1,
    tiSettings2,
    tiSystemData,

    tiCount
};

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportDialog(DialogKind kind, QString title);
    ~ReportDialog() override;
    void setKind(DialogKind kind);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_bClose_clicked();
    void on_bDetails_clicked();

private:
    void addReference(int index, const QString data);
    bool extractLog(TypedIndex index, QString file);

private:
    Ui::Dialog *ui;
    QTemporaryDir *mTempDir = nullptr;
    QSize mOtherSize;
    QStringList mReferences;
    QString mStudioPath;
    QString mSettingsPath;
    int mLogIndex = -1;

};

} // namespace studio
} // namespace gams
#endif // REPORTDIALOG_H
