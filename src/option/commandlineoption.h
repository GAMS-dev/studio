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
#ifndef COMMANDLINEOPTION_H
#define COMMANDLINEOPTION_H

#include<QComboBox>
#include "commandlinetokenizer.h"

namespace gams {
namespace studio {

class CommandLineOption : public QComboBox
{
    Q_OBJECT

public:
    CommandLineOption(bool validateFlag, QWidget* parent);
    ~CommandLineOption();

    QString getCurrentOption() const;

    bool isValidated() const;
    void validated(bool value);

    QString getCurrentContext() const;
    void setCurrentContext(const QString &currentContext);

    void resetCurrentValue();

signals:
    void optionRunChanged();
    void commandLineOptionChanged(QLineEdit* lineEdit, const QString &commandLineStr);

public slots:
    void validateChangedOption(const QString &text);

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;

private:
    QString mCurrentOption;
    QString mCurrentContext;
    int mCurrentIndex;
    bool mValidated;
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEOPTION_H

