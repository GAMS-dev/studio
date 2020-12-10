#include "themewidget.h"
#include "ui_themewidget.h"
#include "logger.h"
#include <QPushButton>
#include <QColorDialog>

namespace gams {
namespace studio {

ThemeWidget::ThemeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThemeWidget)
{
    ui->setupUi(this);
    ui->iconEx->setVisible(false);
}

ThemeWidget::ThemeWidget(Theme::ColorSlot slotFg, QWidget *parent, bool iconExample) :
    QWidget(parent),
    ui(new Ui::ThemeWidget)
{
    ui->setupUi(this);
    ui->iconEx->setVisible(iconExample);
    ui->textFrame->setVisible(!iconExample);
    if (iconExample) {
        mIconEng = new SvgEngine(":/solid/user");
        if (Theme::name(slotFg).startsWith("Disable_")) mIconEng->replaceNormalMode(QIcon::Disabled);
        if (Theme::name(slotFg).startsWith("Active_")) mIconEng->replaceNormalMode(QIcon::Active);
        if (Theme::name(slotFg).startsWith("Select_")) mIconEng->replaceNormalMode(QIcon::Selected);
        ui->iconEx->setIcon(QIcon(mIconEng));
    }

    ui->name->setText(Theme::instance()->text(slotFg));
    setFormatVisible(Theme::hasFontProps(slotFg));
    initSlot(mSlotFg, slotFg, ui->colorFG);
    ui->colorBG1->hide();
    ui->colorBG2->hide();
}

ThemeWidget::ThemeWidget(Theme::ColorSlot slotFg, Theme::ColorSlot slotBg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThemeWidget)
{
    ui->setupUi(this);
    ui->iconEx->setVisible(false);

    setFormatVisible(Theme::hasFontProps(slotFg));
    ui->name->setText(Theme::instance()->text(slotFg ? slotFg : slotBg));
    initSlot(mSlotFg, slotFg, ui->colorFG);
    initSlot(mSlotBg, slotBg, ui->colorBG1);
    ui->colorBG2->hide();
}

ThemeWidget::ThemeWidget(Theme::ColorSlot slotFg, Theme::ColorSlot slotBg, Theme::ColorSlot slotBg2, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThemeWidget)
{
    ui->setupUi(this);
    ui->iconEx->setVisible(false);

    setFormatVisible(Theme::hasFontProps(slotFg));
    ui->name->setText(Theme::instance()->text(slotFg ? slotFg : slotBg));
    initSlot(mSlotFg, slotFg, ui->colorFG);
    initSlot(mSlotBg, slotBg, ui->colorBG1);
    initSlot(mSlotBg2, slotBg2, ui->colorBG2);
}

ThemeWidget::~ThemeWidget()
{
    if (mIconEng) mIconEng->unbind();
    delete ui;
}

void ThemeWidget::initSlot(Theme::ColorSlot &slotVar, const Theme::ColorSlot &slot, QFrame *frame)
{
    slotVar = slot;
    bool active = slot != Theme::invalid;
    frame->setEnabled(active);
    frame->setAutoFillBackground(active);
    if (active) {
        setColor(frame, toColor(slot, mScope), slot==mSlotFg ? 1 : slot==mSlotBg ? 2 : 0);
        if (Theme::hasFontProps(slot)) {
            ui->btBold->setDown(Theme::hasFlag(slot, Theme::fBold));
            ui->btItalic->setDown(Theme::hasFlag(slot, Theme::fItalic));
            QFont font = ui->textEx->font();
            font.setBold(Theme::hasFlag(slot, Theme::fBold));
            font.setItalic(Theme::hasFlag(slot, Theme::fItalic));
            ui->textEx->setFont(font);
        }
        frame->installEventFilter(this);
    }
}

void ThemeWidget::setText(const QString &text)
{
    ui->name->setText(text);
}

QString ThemeWidget::text() const
{
    return ui->name->text();
}

void ThemeWidget::setTextVisible(bool visible)
{
    ui->name->setVisible(visible);
}

void ThemeWidget::setFormatVisible(bool visible)
{
    ui->btBold->setVisible(visible);
    ui->btItalic->setVisible(visible);
}

bool ThemeWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        if (watched == ui->colorFG) selectColor(ui->colorFG, mSlotFg);
        if (watched == ui->colorBG1) selectColor(ui->colorBG1, mSlotBg);
        if (watched == ui->colorBG2) selectColor(ui->colorBG2, mSlotBg2);
    }
    return false;
}

void ThemeWidget::selectColor(QFrame *frame, Theme::ColorSlot slot)
{
    QColorDialog diag;
    diag.setCurrentColor(frame->palette().window().color());
    if (diag.exec()) {
        setColor(frame, diag.currentColor(), slot==mSlotFg ? 1 : slot==mSlotBg ? 2 : 0);
        Theme::setColor(slot, mScope, diag.currentColor());
        emit changed();
    }
}

void ThemeWidget::refresh()
{
    if (mSlotFg) setColor(ui->colorFG, toColor(mSlotFg, mScope), 1);
    if (mSlotBg) setColor(ui->colorBG1, toColor(mSlotBg, mScope), 2);
    if (mSlotBg2) setColor(ui->colorBG2, toColor(mSlotBg2, mScope));
}

void ThemeWidget::setAlignment(Qt::Alignment align)
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

Theme::Scope ThemeWidget::scope() const
{
    return mScope;
}

void ThemeWidget::setScope(const Theme::Scope &scope)
{
    mScope = scope;
}

void ThemeWidget::setColor(QFrame *frame, const QColor &color, int examplePart)
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
