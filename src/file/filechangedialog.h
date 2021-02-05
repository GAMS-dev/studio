#ifndef GAMS_STUDIO_FILECHANGEDIALOG_H
#define GAMS_STUDIO_FILECHANGEDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace gams {
namespace studio {

namespace Ui {
class FileChangeDialog;
}


class FileChangeDialog : public QDialog
{
    Q_OBJECT
public:
    enum class Result {
        rClose,
        rReload,
        rReloadAlways,
        rKeep,
        rCount
    };
    Q_ENUM(Result)

public:
    explicit FileChangeDialog(QWidget *parent = nullptr);
    ~FileChangeDialog() override;

    void show(QString filePath, bool deleted, bool modified, int count);
    bool isForAll();
    static Result enumResult(int result) { return Result(result % int(Result::rCount)); }
    static bool isForAll(int result) { return result >= int(Result::rCount); }
    static bool isAutoReload(int result) { return ((result+1) % int(Result::rCount)) == 3; }

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private slots:
    void buttonClicked();

private:
    Ui::FileChangeDialog *ui;
    QVector<QAbstractButton*> mButtons;

};


} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_FILECHANGEDIALOG_H
