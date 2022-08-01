/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#ifndef CONNECTEDITOR_H
#define CONNECTEDITOR_H

#include "abstractview.h"
#include "connect/connect.h"

namespace gams {
namespace studio {

class MainWindow;

namespace connect {

namespace Ui {
class ConnectEditor;
}

class Connect;

class ConnectEditor : public AbstractView
{
    Q_OBJECT

public:
    explicit ConnectEditor(const QString& connectDataFileName, QWidget *parent = nullptr);
    ~ConnectEditor() override;

private slots:
    void schemaClicked(const QModelIndex &modelIndex);
    void schemaDoubleClicked(const QModelIndex &modelIndex);
    void updateDataColumnSpan();

private:
    Ui::ConnectEditor *ui;

    bool init();

    Connect*     mConnect;
    ConnectData* mData;
    QString mLocation;
};

}
}
}

#endif // CONNECTEDITOR_H
