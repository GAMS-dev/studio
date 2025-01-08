/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "editors/abstractsystemlogger.h"

namespace gams {
namespace studio {

class SystemLogHighlighter;

class SystemLogEdit : public AbstractEdit, public AbstractSystemLogger
{
    Q_OBJECT

public:
    SystemLogEdit(QWidget *parent = nullptr);
    void append(const QString &msg, LogMsgType type = LogMsgType::Warning) override;

    EditorType type() const override;

signals:
    void newMessage(bool focus);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    QString level(LogMsgType type);
    bool selectEntry(QPoint mousePos, bool checkOnly = false);

private:
    SystemLogHighlighter *mHighlighter;
};

}
}

#endif // SYSTEMLOGEDIT_H
