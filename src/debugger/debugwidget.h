/*
 * This file is part of the GAMS Studio project.
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
#ifndef GAMS_STUDIO_DEBUGGER_DEBUGWIDGET_H
#define GAMS_STUDIO_DEBUGGER_DEBUGWIDGET_H

#include <QWidget>
#include "server.h"

namespace gams {
namespace studio {
namespace debugger {

namespace Ui {
class DebugWidget;
}

class Server;

class DebugWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DebugWidget(QWidget *parent = nullptr);
    ~DebugWidget();

    void setText(const QString &text);
    void setDebugServer(Server *server);

signals:
    void sendRun();
    void sendStepLine();
    void sendPause();

private slots:
    void on_tbRun_clicked();
    void on_tbStep_clicked();
    void on_tbPause_clicked();
    void stateChanged(DebugState state);

private:
    Ui::DebugWidget *ui;
    Server *mServer = nullptr;
};


} // namespace debugger
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_DEBUGGER_DEBUGWIDGET_H
