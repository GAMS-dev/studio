#include "schemewidget.h"
#include "ui_schemewidget.h"
#include "logger.h"
#include <QPushButton>
#include <QColorDialog>

namespace gams {
namespace studio {

SchemeWidget::SchemeWidget(QWidget *parent, Scheme::ColorSlot slotFg, Scheme::ColorSlot slotBg, Scheme::ColorSlot slotBg2) :
    QWidget(parent),
    ui(new Ui::SchemeWidget)
{
    ui->setupUi(this);
    ui->colorFG->installEventFilter(this);
    ui->colorBG1->installEventFilter(this);
    ui->colorBG2->installEventFilter(this);
    if (parent) {
        QVariant var = parent->property("hideName");
        if (var.isValid() && var.canConvert<bool>()) setTextVisible(!var.toBool());
        var = parent->property("hideFormat");
        if (var.isValid() && var.canConvert<bool>()) setFormatVisible(!var.toBool());
    }
    setColorSlot(slotFg, slotBg, slotBg2);
}

SchemeWidget::~SchemeWidget()
{
    delete ui;
}

void SchemeWidget::setColorSlot(const Scheme::ColorSlot slotFG, const Scheme::ColorSlot slotBG, const Scheme::ColorSlot slotBG2)
{
    if (mSlotFg != slotFG) {
        mSlotFg = slotFG;
        if (mSlotFg != Scheme::invalid) setColor(ui->colorFG, toColor(slotFG));
//        else ui->
    }
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
    if (event->type() == QEvent::MouseButtonRelease && watched == ui->colorFG) {
        selectColor(ui->colorFG);
    }
    return false;
}

void SchemeWidget::selectColor(QFrame *frame)
{
    QColorDialog diag;
    diag.setCurrentColor(frame->palette().window().color());
    if (diag.exec()) {
        setColor(frame, diag.currentColor());
        Scheme::setColor(mSlotFg, diag.currentColor());
        emit changed();
    }
}

void SchemeWidget::refresh()
{
    setColor(ui->colorFG, toColor(mSlotFg));
    setColor(ui->colorBG1, toColor(mSlotBg));
    setColor(ui->colorBG2, toColor(mSlotBg2));
}

void SchemeWidget::setAlignment(Qt::Alignment align)
{
    if (align.testFlag(Qt::AlignLeft) || align.testFlag(Qt::AlignJustify)) {
        ui->spLeft->changeSize(10, 10);
    } else {
        ui->spLeft->changeSize(10, 10, QSizePolicy::MinimumExpanding);
    }
    if (align.testFlag(Qt::AlignRight) || align.testFlag(Qt::AlignJustify)) {
        ui->spRight->changeSize(10, 10);
    } else {
        ui->spRight->changeSize(10, 10, QSizePolicy::MinimumExpanding);
    }
}

void SchemeWidget::setColor(QFrame *frame, const QColor &color)
{
    QPalette pal = ui->colorFrame->palette();
    pal.setColor(QPalette::Window, color);
    ui->colorFrame->setPalette(pal);
}

} // namespace studio
} // namespace gams
