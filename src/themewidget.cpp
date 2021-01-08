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
//    ui->textFrame->setVisible(!iconExample);
    ui->textFrame->setVisible(false);
    if (iconExample) {
        mIconEng = new SvgEngine(":/solid/user");
        if (Theme::name(slotFg).startsWith("Disable_")) mIconEng->replaceNormalMode(QIcon::Disabled);
        if (Theme::name(slotFg).startsWith("Active_")) mIconEng->replaceNormalMode(QIcon::Active);
        if (Theme::name(slotFg).startsWith("Select_")) mIconEng->replaceNormalMode(QIcon::Selected);
        ui->iconEx->setIcon(QIcon(mIconEng));
    }

    ui->name->setText(Theme::instance()->text(slotFg) + ' ');
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
    ui->name->setText(Theme::instance()->text(slotFg ? slotFg : slotBg) + ' ');
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
    ui->name->setText(Theme::instance()->text(slotFg ? slotFg : slotBg) + ' ');
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
    frame->setVisible(true);
    frame->setEnabled(active);
    frame->setAutoFillBackground(active);
    if (active) {
        if (Theme::hasFontProps(mSlotFg)) {
            connect(ui->btBold, &QAbstractButton::clicked, this, &ThemeWidget::fontFlagsChanged);
            connect(ui->btItalic, &QAbstractButton::clicked, this, &ThemeWidget::fontFlagsChanged);
        }
        refresh();
        frame->installEventFilter(this);
    }
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
    if (event->type() == QEvent::Close && watched == mColorDialog) {
        mColorDialog->deleteLater();
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        if (watched == ui->colorFG) showColorSelector(ui->colorFG);
        else if (watched == ui->colorBG1) showColorSelector(ui->colorBG1);
        else if (watched == ui->colorBG2) showColorSelector(ui->colorBG2);
    }
    return false;
}

void ThemeWidget::showColorSelector(QFrame *frame)
{
    if (mReadonly) return;
    if (!mColorDialog) {
        mColorDialog = new QColorDialog(this->nativeParentWidget());
        mColorDialog->setModal(true);
        mColorDialog->installEventFilter(this);
    }
    mSelectedFrame = frame;
    mColorDialog->setCurrentColor(frame->palette().window().color());
    connect(mColorDialog, &QColorDialog::colorSelected, this, &ThemeWidget::colorChanged);
    mColorDialog->show();
}

void ThemeWidget::colorChanged(const QColor &color)
{
    Theme::ColorSlot slot = mSelectedFrame==ui->colorFG ? mSlotFg : mSelectedFrame==ui->colorBG1 ? mSlotBg : mSlotBg2;
    Theme::setColor(slot, color);
    refresh();
    emit changed();
}

void ThemeWidget::fontFlagsChanged()
{
    if (mReadonly) {
        refresh();
        return;
    }
    Theme::FontFlag flag = ui->btBold->isChecked() ? (ui->btItalic->isChecked() ? Theme::fBoldItalic : Theme::fBold)
                                                   : (ui->btItalic->isChecked() ? Theme::fItalic : Theme::fNormal);
    Theme::setFlags(mSlotFg, flag);

    refresh();
    emit changed();
}

void ThemeWidget::refresh()
{
    if (mSlotFg) {
        setColor(ui->colorFG, toColor(mSlotFg), 1);
        if (Theme::hasFontProps(mSlotFg)) {
            ui->btBold->setChecked(Theme::hasFlag(mSlotFg, Theme::fBold));
            ui->btItalic->setChecked(Theme::hasFlag(mSlotFg, Theme::fItalic));
            QFont font = ui->name->font();
            font.setBold(Theme::hasFlag(mSlotFg, Theme::fBold));
            font.setItalic(Theme::hasFlag(mSlotFg, Theme::fItalic));
            ui->name->setFont(font);
        }
    }
    if (mSlotBg) setColor(ui->colorBG1, toColor(mSlotBg), 2);
    if (mSlotBg2) setColor(ui->colorBG2, toColor(mSlotBg2));

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

void ThemeWidget::setReadonly(bool readonly)
{
    mReadonly = readonly;
    setStyleSheet( readonly ? "color:"+toColor(Theme::Disable_Gray).name()+";" : QString() );
    setMouseTracking(!readonly);
    refresh();
}

void ThemeWidget::setColor(QFrame *frame, const QColor &color, int examplePart)
{
    frame->setStyleSheet("background:"+color.name()+";");
    if (examplePart) {
        if (ui->iconEx->isVisible()) {
            // TODO(JM) colorize Icon
            ui->iconEx->setIcon(ui->iconEx->icon());
        } else {
            ui->name->setStyleSheet((examplePart == 1 ? "color:" : "background") +color.name()+";");
        }

    }
}

} // namespace studio
} // namespace gams

