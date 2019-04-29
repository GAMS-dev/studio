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

    ui->tabWidget->tabBar()->setStyle( new reference::ReferenceTabStyle );

    ui->overrideExistingOptionCheckBox->setCheckState(Qt::Checked);

    ui->eolCommentWidget->setVisible(!eolchars.isEmpty());
    ui->addEOLCommentCheckBox->setVisible(!eolchars.isEmpty());
    if (!eolchars.isEmpty()) {
        for(int i=0; i<eolchars.size(); i++)
           ui->eolCommentCharComboBox->addItem(eolchars.at(i));
        mDefaultEOLChar = mEOLChars.at(0);
    } else {
        mDefaultEOLChar = QChar();
    }
    ui->defaultSeparatorLineEdit->setText(mDefaultSeparator);
    ui->defaultStringquoteLineEdit->setText(mDefaultStringQute);
    connect(ui->eolCommentCharComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index){ emit EOLCharChanged(ui->eolCommentCharComboBox->itemText(index).at(0)); });
//    connect(ui->separatorCharComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
//        [=](int index){ emit separatorCharChanged(ui->separatorCharComboBox->itemText(index).at(0)); });
}

SolverOptionSetting::~SolverOptionSetting()
{
    delete ui;
}

//QChar SolverOptionSetting::getDefaultSeparatorCharacter() const
//{
//    return mDefaultSeparator;
//}

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
