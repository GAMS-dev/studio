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
#include "solveroptionsetting.h"
#include "ui_solveroptionsetting.h"
#include "reference/referencetabstyle.h"

namespace gams {
namespace studio {
namespace option {

SolverOptionSetting::SolverOptionSetting(QString eolchars, QString separator, QString stringquote, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SolverOptionSetting),
    mEOLChars(eolchars),
    mDefaultSeparator(separator),
    mDefaultStringQute(stringquote)
{
    ui->setupUi(this);

    ui->overrideExistingOptionCheckBox->setCheckState(Qt::Checked);
    ui->addEOLCommentCheckBox->setVisible(!eolchars.isEmpty());
    if (!eolchars.isEmpty()) {
        mDefaultEOLChar = mEOLChars.at(0);
    } else {
        mDefaultEOLChar = QChar();
    }
}

SolverOptionSetting::~SolverOptionSetting()
{
    delete ui;
}

QChar SolverOptionSetting::getDefaultEOLCharacter() const
{
    return mDefaultEOLChar;
}

void SolverOptionSetting::on_overrideExistingOptionCheckBox_stateChanged(int checkState)
{
    emit overrideExistingOptionChanged(checkState);
}

void SolverOptionSetting::on_addCommentAboveCheckBox_stateChanged(int checkState)
{
    emit addCommentAboveChanged(checkState);
}

void SolverOptionSetting::on_addEOLCommentCheckBox_stateChanged(int checkState)
{
    emit addOptionDescriptionAsComment(checkState);
}

}
}
}
