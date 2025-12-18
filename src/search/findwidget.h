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

enum FindOption {
    foFocusEdit,
    foFocusTerm,
    foBackwards,
    foContinued,
};
typedef QFlags<FindOption> FindOptions;

class FindWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindWidget(QWidget *parent = nullptr);
    ~FindWidget();
    void setEditWidget(QWidget *widget);
    bool isActive() const;
    void setActive(bool newActive);
    void setLastMatch(const QString &text, size_t pos);
    bool checkLastMatch(const QString &text, size_t pos);
    QString getFindText() const;
    bool setFindText(const QString &text);
    void setReadonly(bool readonly = true);
    QRegularExpression termRexEx();
    QTextDocument::FindFlags findFlags(bool backwards = false);
    void triggerFind(FindOptions options = FindOptions());
    QString replacementText() const;

signals:

protected:
    void focusInEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void on_bClose_clicked();
    void on_bNext_clicked();
    void on_bPrev_clicked();
    void on_bReplace_clicked();
    void on_bReplaceForward_clicked();
    void on_bReplaceBackward_clicked();
    void on_edFind_textEdited(const QString &term);

private:
    bool replace();
    bool doFind(FindOptions options);

private:
    Ui::FindWidget *ui;
    QWidget *mEdit = nullptr;
    bool mActive = false;
    QString mLastMatch;
    size_t mLastPos = 0;
};

} // namespace find
} // namespace studio
} // namespace gams
#endif // FINDWIDGET_H
