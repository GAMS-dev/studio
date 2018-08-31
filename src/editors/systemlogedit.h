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
#ifndef SYSTEMLOGEDIT_H
#define SYSTEMLOGEDIT_H

#include "abstractedit.h"

namespace gams {
namespace studio {

class SystemLogHighlighter;

enum class LogMsgType { Error, Warning, Info };

class SystemLogEdit : public AbstractEdit
{
public:
    SystemLogEdit(QWidget *parent);
    void appendLog(const QString &msg, LogMsgType type = LogMsgType::Warning);

    EditorType type() override;

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QString level(LogMsgType type);

private:
    SystemLogHighlighter *mHighlighter;
};

}
}

#endif // SYSTEMLOGEDIT_H
