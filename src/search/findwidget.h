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
#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>
#include <QRegularExpression>
#include <QTextDocument>

namespace gams {
namespace studio {
namespace find {

namespace Ui {
class FindWidget;
}

class FindWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindWidget(QWidget *parent = nullptr);
    ~FindWidget();
    bool active() const;
    void setActive(bool newActive);
    void setLastMatch(const QString &text);
    QString getFindText() const;
    void setFindText(const QString &text);
    void setReadonly(bool readonly = true);
    QRegularExpression termRexEx();
    QTextDocument::FindFlags findFlags(bool backwards = false);
    void triggerFind(bool backwards = false);

signals:
    void find(const QRegularExpression &rex, QTextDocument::FindFlags options = QTextDocument::FindFlags());

protected:
    void focusInEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void on_bClose_clicked();

    void on_bNext_clicked();

    void on_bPrev_clicked();

    void on_bReplace_clicked();

private:
    Ui::FindWidget *ui;
    bool mActive = false;
    QString mLastMatch;
};

} // namespace find
} // namespace studio
} // namespace gams
#endif // FINDWIDGET_H
