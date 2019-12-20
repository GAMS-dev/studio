#include "schemewidget.h"
#include "ui_schemewidget.h"
#include "logger.h"
#include <QPushButton>
#include <QColorDialog>

namespace gams {
namespace studio {

SchemeWidget::SchemeWidget(QWidget *parent, Scheme::ColorSlot slot) :
    QWidget(parent),
    ui(new Ui::SchemeWidget)
{
    ui->setupUi(this);
    ui->colorFrame->installEventFilter(this);
    if (parent) {
        QVariant var = parent->property("hideName");
        if (var.isValid() && var.canConvert<bool>()) setTextVisible(!var.toBool());
        var = parent->property("hideFormat");
        if (var.isValid() && var.canConvert<bool>()) setFormatVisible(!var.toBool());
    }
    setColorSlot(slot);
}

SchemeWidget::~SchemeWidget()
{
    delete ui;
}

void SchemeWidget::setColorSlot(const Scheme::ColorSlot slot)
{
    if (mSlot == slot) return;
    mSlot = slot;
    setColor(toColor(slot));
}

void SchemeWidget::setText(const QString &text)
{
    ui->name->setText(text);
}

QString SchemeWidget::text() const
{
    return ui->name->text();
}

void SchemeWidget::setTextVisible(bool visible)
{
    ui->name->setVisible(visible);
}

void SchemeWidget::setFormatVisible(bool visible)
{
    ui->btBold->setVisible(visible);
    ui->btItalic->setVisible(visible);
}

bool SchemeWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease && watched == ui->colorFrame) {
        selectColor();
    }
    return false;
}

void SchemeWidget::selectColor()
{
    QColorDialog diag;
    diag.setCurrentColor(ui->colorFrame->palette().window().color());
    if (diag.exec()) {
        setColor(diag.currentColor());
        emit changed();
    }
}

void SchemeWidget::setColor(const QColor &color)
{
    QPalette pal = ui->colorFrame->palette();
    pal.setColor(QPalette::Window, color);
    ui->colorFrame->setPalette(pal);
}

} // namespace studio
} // namespace gams
