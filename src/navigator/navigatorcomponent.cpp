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
#include "navigatorcomponent.h"

namespace gams {
namespace studio {

NavigatorComponent::NavigatorComponent(QObject *parent, MainWindow* main)
    : QObject(parent), mMain(main),
      mNavModel(new NavigatorModel(this, mMain)), mFilterModel(new QSortFilterProxyModel(this))
{
    mFilterModel->setSourceModel(mNavModel);
    fillContent();
}

NavigatorComponent::~NavigatorComponent()
{
    mNavModel->deleteLater();
}

NavigatorModel *NavigatorComponent::Model()
{
    return mNavModel;
}

QSortFilterProxyModel* NavigatorComponent::FilterModel()
{
    return mFilterModel;
}

void NavigatorComponent::setInput(const QString &input)
{
    mFilterModel->setFilterWildcard(input);
}

void NavigatorComponent::fillContent()
{
    QMap<QString, QString> content;
    foreach (FileMeta* fm, mMain->fileRepo()->fileMetas())
        content.insert(fm->location(), "known files");

    foreach (FileMeta* fm, mMain->fileRepo()->openFiles())
        content.insert(fm->location(), "open Files");

    mNavModel->setContent(content);
}



}
}
