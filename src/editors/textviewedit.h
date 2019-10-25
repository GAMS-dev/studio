/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#ifndef TEXTVIEWEDIT_H
#define TEXTVIEWEDIT_H

#include "codeedit.h"
#include "textmapper.h"
#include "locators/settingslocator.h"
#include <QWidget>

namespace gams {
namespace studio {

class TextViewEdit : public CodeEdit
{
    Q_OBJECT
public:
    TextViewEdit(TextMapper &mapper, QWidget *parent = nullptr);
    bool showLineNr() const override { return false; }
    void protectWordUnderCursor(bool protect);
    bool hasSelection() const override;

signals:
    void keyPressed(QKeyEvent *event);
    void updatePosAndAnchor();

public slots:
    void copySelection() override;
    void selectAllText() override;

protected:
    friend class TextView;

    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void recalcWordUnderCursor() override;
    int effectiveBlockNr(const int &localBlockNr) const override;
    void extraSelCurrentLine(QList<QTextEdit::ExtraSelection> &selections) override;

private:
    int topVisibleLine() override;

private:
    TextMapper &mMapper;
    StudioSettings *mSettings;
    qint64 mTopByte = 0;
    int mSubOffset = 0;
    int mDigits = 3;
    bool mKeepWordUnderCursor = false;
};

} // namespace studio
} // namespace gams

#endif // TEXTVIEWEDIT_H
