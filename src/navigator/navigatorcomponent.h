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
 */
#ifndef NAVIGATORCOMPONENT_H
#define NAVIGATORCOMPONENT_H

#include <QObject>
#include <QSortFilterProxyModel>
#include "navigatormodel.h"

namespace gams {
namespace studio {

class MainWindow;
class NavigatorComponent : public QObject
{
    Q_OBJECT
public:
    explicit NavigatorComponent(QObject *parent = nullptr, MainWindow* main = nullptr);
    ~NavigatorComponent();
    NavigatorModel* Model();
    QSortFilterProxyModel* FilterModel();
    void setInput(const QString& input);

private:
    void fillContent();

    MainWindow* mMain = nullptr;
    NavigatorModel* mNavModel = nullptr;
    QSortFilterProxyModel* mFilterModel = nullptr;
};

}
}
#endif // NAVIGATORCOMPONENT_H
