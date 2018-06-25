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
#ifndef ABSTRACTEDIT_H
#define ABSTRACTEDIT_H

#include <QPlainTextEdit>

namespace gams {
namespace studio {

class StudioSettings;

class AbstractEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    enum EditorType { CodeEdit, ProcessLog, SystemLog };

public:
    virtual ~AbstractEdit() override;
    virtual EditorType type() = 0;
    virtual void setOverwriteMode(bool overwrite);
    virtual bool overwriteMode() const;

    bool event(QEvent *event) override;

public slots:
    void afterContentsChanged(int, int, int);

protected:
    AbstractEdit(StudioSettings *settings, QWidget *parent);
    QMimeData* createMimeDataFromSelection() const override;

protected:
    StudioSettings *mSettings = nullptr;
};

}
}

#endif // ABSTRACTEDIT_H
