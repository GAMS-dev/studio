#include "filechangedialog.h"
#include "ui_filechangedialog.h"
#include "../logger.h"
#include <QKeyEvent>

namespace gams {
namespace studio {

FileChangeDialog::FileChangeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileChangeDialog)
{
    ui->setupUi(this);
    ui->bKeep->setAutoDefault(true);
    mButtons << ui->bClose << ui->bReloadAlways << ui->bReload << ui->bKeep;
    ui->bClose->setProperty("value", int(Result::rClose));
    ui->bReload->setProperty("value", int(Result::rReload));
    ui->bReloadAlways->setProperty("value", int(Result::rReloadAlways));
    ui->bKeep->setProperty("value", int(Result::rKeep));
    for (QAbstractButton *button : qAsConst(mButtons)) {
        connect(button, &QAbstractButton::clicked, this, &FileChangeDialog::buttonClicked);
    }
}

FileChangeDialog::~FileChangeDialog()
{
    delete ui;
}

void FileChangeDialog::show(QString filePath, bool deleted, bool modified, int count)
{
    ui->cbAll->setChecked(false);
    setWindowTitle(QString("File %1").arg(deleted ? "vanished" : "changed"));
    QString file("\"" + filePath + "\"");
    QString text(file + (deleted ? "%1 doesn't exist anymore."
                                 : (count>1 ? "%1 have been modified externally."
                                            : "%1 has been modified externally.")));
    text = text.arg(count<2? "" : QString(" and %1 other file%2").arg(count-1).arg(count<3? "" : "s"));
    text += "\nDo you want to %1?";
    if (deleted) text = text.arg("keep the file in editor");
    else if (modified) text = text.arg("reload the file or keep your changes");
    else text = text.arg("reload the file");
    ui->text->setText(text);
    mButtons.at(0)->setVisible(deleted);
    mButtons.at(1)->setVisible(!deleted);
    mButtons.at(2)->setVisible(!deleted);
    ui->wAll->setVisible(count > 1);
    ui->laAll->installEventFilter(this);

    resize(300,120);
    QDialog::show();
}

bool FileChangeDialog::isForAll()
{
    return ui->cbAll->isChecked();
}

void FileChangeDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_A) ui->cbAll->toggle();
    QDialog::keyPressEvent(event);
}

bool FileChangeDialog::eventFilter(QObject *o, QEvent *e)
{
    if (o == ui->laAll && e->type() == QEvent::MouseButtonPress)
        ui->cbAll->toggle();
    return QDialog::eventFilter(o, e);
}

void FileChangeDialog::buttonClicked()
{
    setResult(sender()->property("value").toInt() + (isForAll() ? 4 : 0));
    done(result());
}

} // namespace studio
} // namespace gams


