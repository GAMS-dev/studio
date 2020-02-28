#include "schemewidget.h"
#include "ui_schemewidget.h"
#include "logger.h"
#include <QPushButton>
#include <QColorDialog>

namespace gams {
namespace studio {

SchemeWidget::SchemeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SchemeWidget)
{
    ui->setupUi(this);
    ui->iconEx->setVisible(false);
}

SchemeWidget::SchemeWidget(Scheme::ColorSlot slotFg, QWidget *parent, bool iconExample) :
    QWidget(parent),
    ui(new Ui::SchemeWidget)
{
    ui->setupUi(this);
    ui->iconEx->setVisible(iconExample);
    ui->textFrame->setVisible(!iconExample);
    if (iconExample) {
        mIconEng = new SvgEngine(Scheme::name(slotFg).endsWith("_Line") ? ":/thin/user" : ":/solid/user");
        if (Scheme::name(slotFg).startsWith("Disable_")) mIconEng->replaceNormalMode(QIcon::Disabled);
        if (Scheme::name(slotFg).startsWith("Active_")) mIconEng->replaceNormalMode(QIcon::Active);
        if (Scheme::name(slotFg).startsWith("Select_")) mIconEng->replaceNormalMode(QIcon::Selected);
        ui->iconEx->setIcon(QIcon(mIconEng));
    }

    ui->name->setText(Scheme::instance()->text(slotFg));
    setFormatVisible(Scheme::hasFontProps(slotFg));
    initSlot(mSlotFg, slotFg, ui->colorFG);
    ui->colorBG1->hide();
    ui->colorBG2->hide();
}

SchemeWidget::SchemeWidget(Scheme::ColorSlot slotFg, Scheme::ColorSlot slotBg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SchemeWidget)
{
    ui->setupUi(this);
    ui->iconEx->setVisible(false);

    setFormatVisible(Scheme::hasFontProps(slotFg));
    ui->name->setText(Scheme::instance()->text(slotFg ? slotFg : slotBg));
    initSlot(mSlotFg, slotFg, ui->colorFG);
    initSlot(mSlotBg, slotBg, ui->colorBG1);
    ui->colorBG2->hide();
}

SchemeWidget::SchemeWidget(Scheme::ColorSlot slotFg, Scheme::ColorSlot slotBg, Scheme::ColorSlot slotBg2, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SchemeWidget)
{
    ui->setupUi(this);
    ui->iconEx->setVisible(false);

    setFormatVisible(Scheme::hasFontProps(slotFg));
    ui->name->setText(Scheme::instance()->text(slotFg ? slotFg : slotBg));
    initSlot(mSlotFg, slotFg, ui->colorFG);
    initSlot(mSlotBg, slotBg, ui->colorBG1);
    initSlot(mSlotBg2, slotBg2, ui->colorBG2);
}

SchemeWidget::~SchemeWidget()
{
    if (mIconEng) mIconEng->unbind();
    delete ui;
}

void SchemeWidget::initSlot(Scheme::ColorSlot &slotVar, const Scheme::ColorSlot &slot, QFrame *frame)
{
    slotVar = slot;
    bool active = slot != Scheme::invalid;
    frame->setEnabled(active);
    frame->setAutoFillBackground(active);
    if (active) {
        setColor(frame, toColor(slot, mScope), slot==mSlotFg ? 1 : slot==mSlotBg ? 2 : 0);
        if (Scheme::hasFontProps(slot)) {
            ui->btBold->setDown(Scheme::hasFlag(slot, Scheme::fBold));
            ui->btItalic->setDown(Scheme::hasFlag(slot, Scheme::fItalic));
            QFont font = ui->textEx->font();
            font.setBold(Scheme::hasFlag(slot, Scheme::fBold));
            font.setItalic(Scheme::hasFlag(slot, Scheme::fItalic));
            ui->textEx->setFont(font);
        }
        frame->installEventFilter(this);
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
    if (event->type() == QEvent::MouseButtonRelease) {
        if (watched == ui->colorFG) selectColor(ui->colorFG, mSlotFg);
        if (watched == ui->colorBG1) selectColor(ui->colorBG1, mSlotBg);
        if (watched == ui->colorBG2) selectColor(ui->colorBG2, mSlotBg2);
    }
    return false;
}

void SchemeWidget::selectColor(QFrame *frame, Scheme::ColorSlot slot)
{
    QColorDialog diag;
    diag.setCurrentColor(frame->palette().window().color());
    if (diag.exec()) {
        setColor(frame, diag.currentColor(), slot==mSlotFg ? 1 : slot==mSlotBg ? 2 : 0);
        Scheme::setColor(slot, mScope, diag.currentColor());
        emit changed();
    }
}

void SchemeWidget::refresh()
{
    if (mSlotFg) setColor(ui->colorFG, toColor(mSlotFg, mScope), 1);
    if (mSlotBg) setColor(ui->colorBG1, toColor(mSlotBg, mScope), 2);
    if (mSlotBg2) setColor(ui->colorBG2, toColor(mSlotBg2, mScope));
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

Scheme::Scope SchemeWidget::scope() const
{
    return mScope;
}

void SchemeWidget::setScope(const Scheme::Scope &scope)
{
    mScope = scope;
}

void SchemeWidget::setColor(QFrame *frame, const QColor &color, int examplePart)
{
    QPalette pal = frame->palette();
    pal.setColor(QPalette::Window, color);
    frame->setPalette(pal);
    if (examplePart) {
        if (ui->textFrame->isVisible()) {
            pal = ui->textEx->palette();
            pal.setColor(examplePart == 1 ? QPalette::WindowText : QPalette::Window, color);
            ui->textEx->setPalette(pal);
        }
        if (ui->iconEx->isVisible()) {
            // TODO(JM) colorize Icon
            ui->iconEx->setIcon(ui->iconEx->icon());
        }
    }
}

} // namespace studio
} // namespace gams
