/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "msgbox.h"
#include <QDialog>
#include <QPushButton>

namespace gams {
namespace studio {

MsgBox::MsgBox(QObject *parent)
    : QObject{parent}
{}

int MsgBox::question(const QString &title, const QString &text, QWidget *parent,
                     const QString &button0Text, const QString &button1Text, const QString &button2Text,
                     int defaultButtonNumber, int escapeButtonNumber)
{
    return dialog(QMessageBox::Question, title, text, "", "", parent,
                  button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
}

int MsgBox::question(const QString &title, const QString &text, const QString &informativeText, QWidget *parent,
                     const QString &button0Text, const QString &button1Text, const QString &button2Text,
                     int defaultButtonNumber, int escapeButtonNumber)
{
    return dialog(QMessageBox::Question, title, text, informativeText, "", parent,
                  button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
}

int MsgBox::question(const QString &title, const QString &text, const QString &informativeText, const QString &detailText,
                     QWidget *parent, const QString &button0Text, const QString &button1Text, const QString &button2Text,
                     int defaultButtonNumber, int escapeButtonNumber)
{
    return dialog(QMessageBox::Question, title, text, informativeText, detailText, parent,
                  button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
}

int MsgBox::warning(const QString &title, const QString &text, QWidget *parent,
                    const QString &button0Text, const QString &button1Text, const QString &button2Text,
                    int defaultButtonNumber, int escapeButtonNumber)
{
    return dialog(QMessageBox::Warning, title, text, "", "", parent,
                  button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
}

int MsgBox::warning(const QString &title, const QString &text, const QString &informativeText, QWidget *parent,
                    const QString &button0Text, const QString &button1Text, const QString &button2Text,
                    int defaultButtonNumber, int escapeButtonNumber)
{
    return dialog(QMessageBox::Warning, title, text, informativeText, "", parent,
                  button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
}


int MsgBox::dialog(QMessageBox::Icon icon, const QString &title, const QString &text, const QString &informativeText,
                   const QString &detailText, QWidget *parent,
                   const QString &button0Text, const QString &button1Text, const QString &button2Text,
                   int defaultButtonNumber, int escapeButtonNumber)
{
    QMessageBox messageBox(icon, title, text, QMessageBox::NoButton, parent);
    messageBox.setInformativeText(informativeText);
    messageBox.setDetailedText(detailText);
    QList<QPushButton*> buttonList;

    buttonList << messageBox.addButton(button0Text.isEmpty() ? "OK" : button0Text, QMessageBox::ActionRole);
    if (!button1Text.isEmpty()) buttonList << messageBox.addButton(button1Text, QMessageBox::ActionRole);
    if (!button2Text.isEmpty()) buttonList << messageBox.addButton(button2Text, QMessageBox::ActionRole);

    messageBox.setDefaultButton(static_cast<QPushButton *>(buttonList.value( defaultButtonNumber)));
    messageBox.setEscapeButton(buttonList.value(escapeButtonNumber));
    messageBox.exec();

    return buttonList.indexOf(messageBox.clickedButton());
}

} // namespace studio
} // namespace gams
