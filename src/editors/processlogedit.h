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
#ifndef PROCESSLOGEDIT_H
#define PROCESSLOGEDIT_H

#include "abstractedit.h"

namespace gams {
namespace studio {

class ProcessLogEdit : public AbstractEdit
{
    Q_OBJECT

public:
    ProcessLogEdit(QWidget *parent = nullptr);
    EditorType type() override;

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void jumpToLst(QPoint pos, bool fuzzy);
    void contextMenuEvent(QContextMenuEvent *e) override;
    void extraSelCurrentLine(QList<QTextEdit::ExtraSelection> &selections) override;

};

}
}

#endif // PROCESSLOGEDIT_H
