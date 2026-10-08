/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "statuswidgets.h"
#include "theme.h"
#include "support/licensebutton.h"

#include <QMainWindow>
#include <QStatusBar>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QLayout>
#include <QStyleOption>
#include <QStringConverter>
#include <QTimer>

using namespace gams::studio::support::LicenseStateEnum;

const QString UC_Checkmark("\u2713");

namespace gams {
namespace studio {

class AmountLabel: public QWidget
{
    qreal mLoadAmount = 1.0;
    QString mLoadingText;
    QString mBaseText;
    QString mFullText;
public:
    explicit AmountLabel(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QWidget(parent, f) { setBaseText(text); initPolicy(); }
    qreal getAmount() const { return mLoadAmount; }
    void setAmount(qreal value);
    void setBaseText(const QString &text);
    void setLoadingText(const QString &loadingText);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void updateText();
    void initPolicy();
};


StatusWidgets::StatusWidgets(QMainWindow *parent) : QObject(parent), mStatusBar(parent->statusBar())
{
    mEditLines = new QLabel("0 lines");
    mStatusBar->addPermanentWidget(mEditLines);
    mEditLines->setMinimumWidth(mEditLines->height()*2);
    mEditLines->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    mEditPosAnsSel = new QLabel("0 / 0");
    mStatusBar->addPermanentWidget(mEditPosAnsSel);
    mEditPosAnsSel->setMinimumWidth(mEditPosAnsSel->height()*2);
    mEditPosAnsSel->setAlignment(Qt::AlignCenter);

    mEditMode = new QLabel("INS");
    mStatusBar->addPermanentWidget(mEditMode);
    mEditMode->setMinimumWidth(int(mEditMode->height()*0.8));
    mEditMode->setAlignment(Qt::AlignCenter);

    mEditEncode = new QLabel("");
    mStatusBar->addPermanentWidget(mEditEncode);
    mEditEncode->setMinimumWidth(mEditEncode->height()*3);

    mLicense = new support::LicenseButton(mStatusBar);
    QHBoxLayout *layout = new QHBoxLayout(mLicense);
    mLicenseIconLabel = new QLabel();
    mLicenseIconLabel->setPixmap(QPixmap(":/solid/new-w"));
    layout->addWidget(mLicenseIconLabel);
    layout->addStretch();

    layout->setContentsMargins(5, 2, 6, 2);
    mLicenseTextLabel = new QLabel("");
    QFont font = mLicenseTextLabel->font();
    font.setPointSize(font.pointSize() + 1);
    font.setBold(true);
    mLicenseTextLabel->setFont(font);
    mLicenseTextLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(mLicenseTextLabel);

    mLicense->setLayout(layout);
    mLicense->setFlat(true);
    mLicense->setMaximumWidth(qRound(mLicense->height() * 1.6));
    mLicense->setMinimumWidth(qRound(mLicense->height() * 1.6));
    mStatusBar->addPermanentWidget(mLicense);
    mLicense->setAutoFillBackground(true);
    connect(mLicense, &QPushButton::clicked, this, [this]() {
        if (qApp->keyboardModifiers().testFlag(Qt::ControlModifier)
            && qApp->keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            setLicenseState(mLicenseState < lsNetCheckoutEnd ? support::LicenseState(mLicenseState+1) : lsNone
                            , mLicenseExpire);
        } else showLicense();
    });
    setLicenseState(lsNone, mLicenseExpire);
    connect(Theme::instance(), &Theme::changed, this, [this]() {
        QTimer::singleShot(0, this, [this](){ setLicenseState(mLicenseState, mLicenseExpire); });
    });

    mFileName = new AmountLabel("Filename");
    mFileName->setLoadingText("(counting)");
    mStatusBar->addWidget(mFileName, 0);
}

void StatusWidgets::setFileName(const QString &fileName)
{
    mFileName->setAmount(1.0);
    mLoadAmount = 1.0;
    mFileName->setBaseText(fileName);
}

void StatusWidgets::setEncoding(const QString &encodingName)
{
    mEditEncode->setText(encodingName);
}

void StatusWidgets::setLineCount(int lines)
{
    if (lines < -1) {
        mEditLines->setText(QString("~%1 lines").arg(-lines));
    } else if (lines >= 0) {
        mEditLines->setText(QString("%1 lines").arg(lines));
    } else {
        mEditLines->setText("            ");
    }
}

void StatusWidgets::setLoadAmount(qreal amount)
{
    mFileName->setAmount(amount);
}

void StatusWidgets::setEditMode(EditMode mode)
{
    switch (mode) {
    case EditMode::Readonly: mEditMode->setText("RO"); break;
    case EditMode::Insert: mEditMode->setText("INS"); break;
    case EditMode::Overwrite: mEditMode->setText("OVR"); break;
    }
}

void StatusWidgets::setPosAndAnchor(QPoint pos, QPoint anchor)
{
    QString posText;
    if (pos.isNull() || pos.y() == -1) {
        posText = "      ";
    } else {
        QString estimated = (pos.y() < 0) ? "~" : "";
        posText = QString("%1%2 / %3").arg(estimated).arg(qAbs(pos.y())).arg(pos.x());
        if (!anchor.isNull() && anchor != pos) {
            estimated = (pos.y()<0 || anchor.y()<0) ? "~" : "";
            QString absLineDiff = QString::number(qAbs(qAbs(pos.y())-qAbs(anchor.y()))+1);
            posText += QString(" (%1%2 / %3)").arg(estimated, absLineDiff).arg(qAbs(pos.x()-anchor.x()));
        }
    }
    mEditPosAnsSel->setText(posText);
}

void StatusWidgets::setLoadingText(const QString &loadingText)
{
    mFileName->setLoadingText(loadingText);
}

void StatusWidgets::setLicenseState(support::LicenseState licenseState, const QDateTime &expire)
{
    Theme::setThemeColorPalette(mStatusBar, false, false);

    QColor background = Qt::white;
    QIcon icon = Theme::icon(":/solid/new-w");
    QIcon icon2;
    QString text;
    QString toolTip;
    QPalette pal = mStatusBar->palette();

    switch (licenseState) {
    case lsChecking:
        background = pal.color(QPalette::Window);
        icon = Theme::icon(":/solid/search");
        toolTip = "Validating GAMS license";
        break;
    case lsMaintenance:
        background = Theme::mixColor(pal.color(QPalette::Window), Theme::Normal_Green, .6);
        icon = Theme::icon(":/solid/computer");
        toolTip = "Local GAMS maintenance license";
        icon2 = Theme::icon(":/solid/tlock-open");
        break;
    case lsMaintenanceInvalid:
        background = Theme::mixColor(QColor(255,255,255), Theme::Normal_Red, .9);
        toolTip = "Local GAMS maintenance license (GAMS version too new)\nSee Settings > General to change the version.";
        icon = Theme::icon(":/solid/computer");
        icon2 = Theme::icon(":/solid/tlock");
        break;
    case lsLocal:
        background = Theme::mixColor(pal.color(QPalette::Window), Theme::Normal_Green, .6);
        icon = Theme::icon(":/solid/computer");
        toolTip = "Local GAMS license";
        text = UC_Checkmark;
        break;
    case lsLocalEnd:
        background = Theme::mixColor(pal.color(QPalette::Window), Theme::Normal_Yellow, .8);
        icon = Theme::icon(":/solid/computer");
        text = "!";
        toolTip = "Local GAMS license (expires soon)";
        break;
    case lsLocalInvalid:
        background = Theme::mixColor(QColor(255,255,255), Theme::Normal_Red, .9);
        icon = Theme::icon(":/solid/computer");
        text = "?";
        toolTip = "Local GAMS license (expired)";
        break;
    case lsNet:
        background = Theme::mixColor(pal.color(QPalette::Window), Theme::Normal_Green, .6);
        icon = Theme::icon(":/solid/internet");
        toolTip = "GAMS network license (validated)";
        text = UC_Checkmark;
        break;
    case lsNetEnd:
        background = Theme::mixColor(pal.color(QPalette::Window), Theme::Normal_Yellow, .8);
        icon = Theme::icon(":/solid/internet");
        text = "!";
        toolTip = "GAMS network license (expires soon)";
        break;
    case lsNetNoConnection:
        background = Theme::mixColor(QColor(255,255,255), Theme::Normal_Red, .9);
        icon = Theme::icon(":/solid/internet-off");
        text = "?";
        toolTip = "GAMS network license (not validated, please check your internet connection)";
        break;
    case lsNetInvalid:
        background = Theme::mixColor(QColor(255,255,255), Theme::Normal_Red, .9);
        icon = Theme::icon(":/solid/internet");
        text = "?";
        toolTip = "GAMS network license (expired)";
        break;
    case lsNetCheckout:
        background = Theme::mixColor(pal.color(QPalette::Window), Theme::Normal_Green, .6);
        icon = Theme::icon(":/solid/plane");
        text = UC_Checkmark;
        toolTip = "GAMS network license (checked out)";
        break;
    case lsNetCheckoutEnd:
        background = Theme::mixColor(pal.color(QPalette::Window), Theme::Normal_Yellow, .8);
        icon = Theme::icon(":/solid/plane");
        text = "!";
        toolTip = "GAMS network license (check out expires soon)";
        break;
    case lsNetCheckoutInvalid:
        background = Theme::mixColor(QColor(255,255,255), Theme::Normal_Red, .9);
        icon = Theme::icon(":/solid/plane");
        text = "?";
        toolTip = "GAMS network license (checkout expired)";
        break;
    default:
        background = Theme::mixColor(QColor(255,255,255), Theme::Normal_Red, .9);
        icon = Theme::icon(":/solid/new-w");
        text = "?";
        toolTip = "GAMS license not found (click for details).";
        break;
    }
    if (expire.isValid()) {
        if (expire.secsTo(QDateTime::currentDateTime()) < 0)
            toolTip += "\nvalid until: " + expire.toString("yyyy-MMM-dd hh:mm");
        else
            toolTip += "\ninvalid since: " + expire.toString("yyyy-MMM-dd hh:mm");
    }

    pal.setColor(QPalette::ButtonText, Qt::white);
    pal.setColor(QPalette::AlternateBase, background);
    mLicense->setPalette(pal);
    mLicenseTextLabel->setPixmap(icon2.pixmap(14,14));
    mLicenseTextLabel->setText(text);
    mLicenseIconLabel->setPixmap(icon.pixmap(14,14));
    mLicense->setToolTip(toolTip);
    mLicenseState = licenseState;
    mLicenseExpire = expire;
}

void AmountLabel::setAmount(qreal value)
{
    mLoadAmount = value;
    updateText();
}

void AmountLabel::setBaseText(const QString &text)
{
    mBaseText = text;
    updateText();
}

void AmountLabel::setLoadingText(const QString &loadingText)
{
    mLoadingText = loadingText;
    updateText();
}

void AmountLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (mFullText.isEmpty()) {
        QWidget::paintEvent(event);
    } else {
        int availableWidth = width();
        if (availableWidth < 0) availableWidth = 0;
        QFontMetrics metrics = painter.fontMetrics();
        QString elidedText = metrics.elidedText(mFullText, Qt::ElideMiddle, availableWidth);
        painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, elidedText);
    }

    if (mLoadAmount < 1.0) {
        int x = qRound((width() - 1) * qBound(0.0 ,mLoadAmount, 1.0));
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(160,160,160, 220));
        painter.drawRect(QRect(x, 0, width()-1, 3));
        painter.setBrush(QColor(255,120,0, 220));
        painter.drawRect(QRect(0, 0, x, 3));
        painter.restore();
    }
}

QSize AmountLabel::minimumSizeHint() const
{
    return QSize(15, 20);
}

QSize AmountLabel::sizeHint() const
{
    if (mFullText.isEmpty())
        return QSize(50, 20);

    QFontMetrics metrics(font());
    return QSize(metrics.horizontalAdvance(mFullText), 20);
}

void AmountLabel::updateText()
{
    mFullText = mBaseText + ((mLoadAmount < 1.0) ? " "+mLoadingText : "");
    setToolTip(mFullText);
    updateGeometry();
    update();

}

void AmountLabel::initPolicy()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumWidth(15);
}

} // namespace Studio
} // namespace gams
