#ifndef GAMS_STUDIO_FILECHANGEDIALOG_H
#define GAMS_STUDIO_FILECHANGEDIALOG_H

#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>

namespace gams {
namespace studio {


class FileChangeDialog : public QMessageBox
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

private slots:
    void buttonClicked();

private:
    QVector<QPushButton*> mButtons;
    QCheckBox *mCbAll;

};


} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_FILECHANGEDIALOG_H
