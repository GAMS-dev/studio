/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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

namespace gams {
namespace studio {

class WpLabel : public QLabel
{
    Q_OBJECT

public:
    WpLabel(QWidget *parent = nullptr);
    WpLabel(const QString &content, const QString &link, QWidget *parent = nullptr);

    void setInactive(bool inactive);

protected:
    void mousePressEvent(QMouseEvent *event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

signals:
    void relayActionLab(QString action);
    void relayModLibLoad(QString lib);
    void relayOpenDoc(QString doc, QString anchor);

private:
    QString mContent;
    QString mLink;
    bool mInactive = false;
};

}
}
#endif // WPLABEL_H
