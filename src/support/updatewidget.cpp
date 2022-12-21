#include "updatewidget.h"
#include "ui_updatewidget.h"
#include "settings.h"

#include <QDate>

namespace gams {
namespace studio {
namespace support {

UpdateWidget::UpdateWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UpdateWidget)
{
    ui->setupUi(this);
    connect(ui->closeButton, &QPushButton::clicked, this, [this]{ hide(); });
    connect(ui->settingsButton, &QPushButton::clicked, this, &UpdateWidget::openSettings);
    connect(ui->remindButton, &QPushButton::clicked, this, &UpdateWidget::remindMeLater);
}

UpdateWidget::~UpdateWidget()
{
    delete ui;
}

void UpdateWidget::setText(const QString &text)
{
    ui->label->setText(text);
}

void UpdateWidget::remindMeLater()
{
    Settings::settings()->setDate(skNextUpdateCheckDate, QDate::currentDate().addDays(1));
    hide();
}

}
}
}
