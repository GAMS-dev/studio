/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_FILTERLINEEDIT_H
#define GAMS_STUDIO_FILTERLINEEDIT_H

#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>

namespace gams {
namespace studio {


class MiniButton : public QPushButton
{
    Q_OBJECT
public:
    MiniButton(QWidget *parent = nullptr): QPushButton(parent) {}
    virtual ~MiniButton() override {}
    QSize sizeHint() const override;
};


class FilterLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    enum FilterLineEditFlag {
        foNone   = 0x00,        // No flag set
        foClear  = 0x01,        // Clear button visible
        foExact  = 0x02,        // Exact button visible (BoundaryMode dependent)
        foRegEx  = 0x04,        // RegEx button visible
        foColumn = 0x08,        // Column button visible
        foCaSens = 0x10,        // Case Sensitivity button visible
    };
    Q_DECLARE_FLAGS(FilterLineEditFlags, FilterLineEditFlag)
    Q_FLAG(FilterLineEditFlags)

    enum BoundaryMode {
        bmLineBound,
        bmWordBound,
    };

public:
    explicit FilterLineEdit(QWidget *parent = nullptr);
    explicit FilterLineEdit(const QString &contents, QWidget *parent = nullptr);
    const QRegularExpression &regExp();
    void setOptionState(FilterLineEditFlag option, int state);
    void setKeyColumn(int column);
    void hideOptions(FilterLineEditFlags options);
    void showOptions(FilterLineEditFlags options);
    int effectiveKeyColumn();
    bool exactMatch();
    bool isRegEx();
    bool isCaseSensitive();
    BoundaryMode docMode() const;
    void setBoundaryMode(BoundaryMode newDocMode);

signals:
    void regExpChanged(QRegularExpression regExp);
    void columnScopeChanged();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void init();
    void updateRegExp();
    QAbstractButton *createButton(const QStringList &iconPaths, const QStringList &toolTips);
    int nextButtonState(QAbstractButton *button, int forceState = -1);
    int buttonState(QAbstractButton *button);
    void updateTextMargins();
    QAbstractButton *button(FilterLineEditFlag option);

private:
    QAbstractButton *mClearButton = nullptr;
    QAbstractButton *mCaseSenseButton = nullptr;
    QAbstractButton *mExactButton = nullptr;
    QAbstractButton *mRegExButton = nullptr;
    QAbstractButton *mAllColButton = nullptr;
    QRegularExpression mRegExp;
    BoundaryMode mBoundaryMode = bmLineBound;
    bool mCanClear = true;
    int mKeyColumn = -1;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_FILTERLINEEDIT_H
