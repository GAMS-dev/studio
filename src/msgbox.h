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
#ifndef MSGBOX_H
#define MSGBOX_H

#include <QMessageBox>

namespace gams {
namespace studio {

class MsgBox : public QObject
{
    Q_OBJECT
public:
    static int question(const QString &title, const QString& text, QWidget *parent,
                        const QString& button0Text, const QString& button1Text = QString(),
                        const QString& button2Text = QString(), int defaultButtonNumber = 0, int escapeButtonNumber = -1);
    static int question(const QString &title, const QString& text, const QString &informativeText, QWidget *parent,
                        const QString& button0Text, const QString& button1Text = QString(),
                        const QString& button2Text = QString(), int defaultButtonNumber = 0, int escapeButtonNumber = -1);
    static int question(const QString &title, const QString& text, const QString &informativeText, const QString &detailText,
                        QWidget *parent, const QString& button0Text, const QString& button1Text = QString(),
                        const QString& button2Text = QString(), int defaultButtonNumber = 0, int escapeButtonNumber = -1);
    static int warning(const QString &title, const QString& text, QWidget *parent,
                       const QString& button0Text, const QString& button1Text = QString(),
                       const QString& button2Text = QString(), int defaultButtonNumber = 0, int escapeButtonNumber = -1);
    static int warning(const QString &title, const QString& text, const QString &informativeText, QWidget *parent,
                       const QString& button0Text, const QString& button1Text = QString(),
                       const QString& button2Text = QString(), int defaultButtonNumber = 0, int escapeButtonNumber = -1);
    static int warning(const QString &title, const QString& text, const QString &informativeText, const QString &detailText,
                       QWidget *parent, const QString& button0Text, const QString& button1Text = QString(),
                       const QString& button2Text = QString(), int defaultButtonNumber = 0, int escapeButtonNumber = -1);

private:
    explicit MsgBox(QObject *parent = nullptr);
    static int dialog(QMessageBox::Icon icon, const QString &title, const QString& text,
                      const QString &informativeText, const QString &detailText,
                      QWidget *parent, const QString& button0Text, const QString& button1Text = QString(),
                      const QString& button2Text = QString(), int defaultButtonNumber = 0, int escapeButtonNumber = -1);

};

} // namespace studio
} // namespace gams

#endif // MSGBOX_H
