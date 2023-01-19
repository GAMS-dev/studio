/*
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
 */
#ifndef UPDATEWIDGET_H
#define UPDATEWIDGET_H

#include <QWidget>

namespace gams {
namespace studio {
namespace support {

namespace Ui {
class UpdateWidget;
}

class UpdateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateWidget(QWidget *parent = nullptr);
    ~UpdateWidget();

    void setText(const QString &text);

signals:
    void openSettings();

private slots:
    void remindMeLater();

private:
    Ui::UpdateWidget *ui;
};

}
}
}

#endif // UPDATEWIDGET_H
