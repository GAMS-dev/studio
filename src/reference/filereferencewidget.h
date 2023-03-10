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
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FILEREFERENCEWIDGET_H
#define FILEREFERENCEWIDGET_H

#include <QWidget>

#include "reference.h"
#include "referenceviewer.h"

namespace Ui {
class FileReferenceWidget;
}

namespace gams {
namespace studio {
namespace reference {

class FileReferenceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileReferenceWidget(Reference* ref, ReferenceViewer *parent = nullptr);
    ~FileReferenceWidget();

private:
    Ui::FileReferenceWidget *ui;

    Reference* mReference;
    ReferenceViewer* mReferenceViewer;

};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // FILEREFERENCEWIDGET_H
