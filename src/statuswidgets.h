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
#ifndef STATUSWIDGETS_H
#define STATUSWIDGETS_H

#include <QObject>
#include <QPoint>
#include <QLabel>

class QStatusBar;
class QMainWindow;
class QLabel;

namespace gams {
namespace studio {


class AmountLabel: public QLabel
{
    Q_OBJECT
    qreal mLoadAmount = 1.0;
    QString mLoadingText;
    QString mBaseText;
public:
    AmountLabel(QWidget *parent) : QLabel(parent) {}
    explicit AmountLabel(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QLabel(parent, f) {}
    explicit AmountLabel(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QLabel(text, parent, f) { setBaseText(text); }
    qreal getAmount() const { return mLoadAmount; }
    void setAmount(qreal value);
    void setBaseText(const QString &text);
    void setLoadingText(const QString &loadingText);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class StatusWidgets : public QObject
{
    Q_OBJECT
public:
    enum class EditMode {Readonly, Insert, Overwrite};
    Q_ENUM(EditMode)

public:
    explicit StatusWidgets(QMainWindow *parent);
    void setFileName(const QString &fileName);
    void setEncoding(const QString &encodingName);
    void setLineCount(int lines);
    void setLoadAmount(qreal amount);
    void setEditMode(EditMode mode);
    void setPosAndAnchor(QPoint pos = QPoint(), QPoint anchor = QPoint());
    void setLoadingText(const QString &loadingText);

private:
    QStatusBar* mStatusBar;
    QLabel* mEditMode = nullptr;
    QLabel* mEditEncode = nullptr;
    QLabel* mEditPosAnsSel = nullptr;
    QLabel* mEditLines = nullptr;
    AmountLabel* mFileName = nullptr;
    QLabel* mProcessInfo = nullptr;
    qreal mLoadAmount = 1.0;

};

} // namespace Studio
} // namespace gams

#endif // STATUSWIDGETS_H
