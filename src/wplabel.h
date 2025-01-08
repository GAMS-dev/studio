/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#ifndef WPLABEL_H
#define WPLABEL_H

#include <QLabel>
#include <QIcon>

namespace gams {
namespace studio {

class WpLabel : public QLabel
{
    Q_OBJECT

public:
    WpLabel(QWidget *parent = nullptr);
    WpLabel(const QString &content, const QString &link, QWidget *parent = nullptr);

    void setInactive(bool inactive);
    void setCloseable(bool closeable = true);

    void setIcon(const QIcon &icon);
    QIcon icon() const { return mIcon; }
    void setIconSize(const QSize &size);
    QSize iconSize() { return mIconSize; }
    void setIconAlignment(Qt::Alignment alignment);
    Qt::Alignment iconAlignment() const { return mAlignment; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void relayActionLab(QString action);
    void relayModLibLoad(QString lib);
    void relayOpenDoc(QString doc, QString anchor);
    void removeFromHistory(const QString &path);

private:
    QString mContent;
    QString mLink;
    bool mInactive = false;
    bool mActive = false;
    bool mCloseable = false;
    QIcon mIcon;
    QSize mIconSize;
    Qt::Alignment mAlignment = Qt::AlignCenter;
    QColor GAMS_ORANGE = QColor(243,150,25);

private:
    void updateMouseOverColor(bool hovered);
    bool inCloseButton(const QPoint &pos);
};

}
}
#endif // WPLABEL_H
