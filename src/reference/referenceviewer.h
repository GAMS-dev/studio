/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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

#ifndef REFERENCEVIEWER_H
#define REFERENCEVIEWER_H

#include <QWidget>
#include <QList>
#include <QMap>
#include <QTabWidget>
#include "common.h"
#include "reference.h"

namespace Ui {
class ReferenceViewer;
}

namespace gams {
namespace studio {
namespace reference {

class Reference;
class ReferenceTabStyle;
class ReferenceViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ReferenceViewer(QString referenceFile, QTextCodec* codec, QWidget *parent = nullptr);
    ~ReferenceViewer();
    void selectSearchField() const;

signals:
    void jumpTo(ReferenceItem item);

public slots:
    void on_referenceFileChanged(QTextCodec* codec);
    void on_tabBarClicked(int index);
    void updateView(bool status);

private:
    Ui::ReferenceViewer *ui;

    QTextCodec *mCodec;

    QScopedPointer<Reference> mReference;
    QScopedPointer<ReferenceTabStyle> mRefTabStyle;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // REFERENCEVIEWER_H
