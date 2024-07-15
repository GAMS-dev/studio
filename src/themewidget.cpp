/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "themewidget.h"
#include "ui_themewidget.h"
#include "logger.h"
#include <QPushButton>
#include <QColorDialog>
#include <QMouseEvent>

namespace gams {
namespace studio {

ThemeWidget::ThemeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThemeWidget)
{
    ui->setupUi(this);
    baseInit();
    ui->iconEx->setVisible(false);
}

ThemeWidget::ThemeWidget(QList<Theme::ColorSlot> colors, QWidget *parent, bool iconExample) :
    QWidget(parent),
    ui(new Ui::ThemeWidget)
{
    ui->setupUi(this);
    baseInit();
    ui->iconEx->setVisible(iconExample);
//    ui->textFrame->setVisible(!iconExample);
    ui->textFrame->setVisible(false);
    if (iconExample) {
        mIconEng = new SvgEngine(":/solid/user");
        if (Theme::name(colors.at(0)).startsWith("Disable_")) mIconEng->replaceNormalMode(QIcon::Disabled);
        if (Theme::name(colors.at(0)).startsWith("Active_")) mIconEng->replaceNormalMode(QIcon::Active);
        if (Theme::name(colors.at(0)).startsWith("Select_")) mIconEng->replaceNormalMode(QIcon::Selected);
        ui->iconEx->setIcon(QIcon(mIconEng));
    }

    ui->name->setText(' ' + Theme::instance()->text(colors.at(0)) + ' ');
    setFormatVisible(Theme::hasFontProps(colors.at(0)));
    initSlot(mSlotFg, colors.at(0), ui->colorFG);
    initSlot(mSlotBg, colors.at(1), ui->colorBG1);
    mHasAutoBackground = true;
    ui->colorBG2->hide();
}

ThemeWidget::ThemeWidget(Theme::ColorSlot slotFg, Theme::ColorSlot slotBg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThemeWidget)
{
    ui->setupUi(this);
    baseInit();
    ui->iconEx->setVisible(false);

    setFormatVisible(Theme::hasFontProps(slotFg));
    ui->name->setText(' ' + Theme::instance()->text(slotFg ? slotFg : slotBg) + ' ');
    initSlot(mSlotFg, slotFg, ui->colorFG);
    initSlot(mSlotBg, slotBg, ui->colorBG1);
    ui->colorBG2->hide();
}

ThemeWidget::ThemeWidget(Theme::ColorSlot slotFg, Theme::ColorSlot slotBg, Theme::ColorSlot slotBg2, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThemeWidget)
{
    ui->setupUi(this);
    baseInit();
    ui->iconEx->setVisible(false);
    ui->textFrame->setVisible(false);

    setFormatVisible(Theme::hasFontProps(slotFg));
    ui->name->setText(' ' + Theme::instance()->text(slotFg ? slotFg : slotBg) + ' ');
    initSlot(mSlotFg, slotFg, ui->colorFG);
    initSlot(mSlotBg, slotBg, ui->colorBG1);
    initSlot(mSlotBg2, slotBg2, ui->colorBG2);
}

ThemeWidget::~ThemeWidget()
{
    if (mIconEng) mIconEng->unbind();
    delete ui;
}

void ThemeWidget::baseInit()
{
    QList<QFrame*> frames;
    frames << ui->colorFG << ui->colorBG1 << ui->colorBG2;
    for (QFrame *frame : std::as_const(frames)) {
        QPalette pal = frame->parentWidget()->palette();
        QColor c = pal.color(QPalette::Button);
        c = (c.black() < 128) ? c.darker(105) : c.lighter(115);
        frame->setStyleSheet(":disabled{background:"+c.name()+";color:"+c.name()+";}");
    }
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
    if (event->type() == QEvent::MouseButtonRelease) {
        if (watched == ui->colorFG)
            showColorSelector(ui->colorFG);
        else if (watched == ui->colorBG1) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (mHasAutoBackground && me->button() == Qt::RightButton)
                colorChanged(Theme::CAutoBackground);
            else showColorSelector(ui->colorBG1);
        } else if (watched == ui->colorBG2)
            showColorSelector(ui->colorBG2);
    }
    return false;
}

void ThemeWidget::showColorSelector(QFrame *frame)
{
    if (mReadonly) return;
    if (!mColorDialog) {
        mColorDialog = new QColorDialog(this);
        mColorDialog->setParent(this);
        mColorDialog->setModal(true);
    }
    mSelectedFrame = frame;
    mColorDialog->setCurrentColor(frame->palette().window().color());
    connect(mColorDialog, &QColorDialog::colorSelected, this, &ThemeWidget::colorChanged);
    connect(mColorDialog, &QColorDialog::finished, this, [this](){
        mColorDialog->deleteLater();
        mColorDialog = nullptr;
    });
    mColorDialog->open();
}

void ThemeWidget::colorChanged(const QColor &color)
{
    Theme::ColorSlot slot = mSelectedFrame==ui->colorFG ? mSlotFg : mSelectedFrame==ui->colorBG1 ? mSlotBg : mSlotBg2;
    QColor old = Theme::color(slot);
    if (old != color) {
        emit aboutToChange();
        Theme::setColor(slot, color);
        refresh();
        emit changed();
    }
}

void ThemeWidget::fontFlagsChanged()
{
    if (mReadonly) {
        refresh();
        return;
    }
    Theme::FontFlag flag = ui->btBold->isChecked() ? (ui->btItalic->isChecked() ? Theme::fBoldItalic : Theme::fBold)
                                                   : (ui->btItalic->isChecked() ? Theme::fItalic : Theme::fNormal);
    emit aboutToChange();
    Theme::setFlags(mSlotFg, flag);
    refresh();
    emit changed();
}

void ThemeWidget::refresh()
{
    baseInit();
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
        if (!ui->iconEx->isVisible()) {
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
    QColor transColor = color;
    if (mHasAutoBackground && color == Theme::CAutoBackground) {
        transColor = Theme::instance()->color(Theme::Edit_background);
    }
    frame->setStyleSheet("background:"+transColor.name()+";");
    if (examplePart) {
        if (ui->iconEx->isVisible()) {
            ui->iconEx->setIcon(ui->iconEx->icon());
        } else {
            QString sheet;
            sheet = "color:" + (mSlotFg != Theme::invalid ? toColor(mSlotFg).name()
                                                          : toColor(Theme::Edit_text).name()) + ";";
            bool useEditBg = mSlotBg == Theme::invalid || (mHasAutoBackground && toColor(mSlotBg) == Theme::CAutoBackground);
            sheet += "background:" + (useEditBg ? toColor(Theme::Edit_background).name() : toColor(mSlotBg).name()) + ";";
            ui->name->setStyleSheet(sheet);
        }
    }
}

} // namespace studio
} // namespace gams

