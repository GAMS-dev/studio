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
#ifndef OPTIONCOMPLETERDELEGATE_H
#define OPTIONCOMPLETERDELEGATE_H

#include <QStyledItemDelegate>
#include "commandlinetokenizer.h"
#include "option.h"

namespace gams {
namespace studio {

class OptionCompleterDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    OptionCompleterDelegate(CommandLineTokenizer* tokenizer, QObject* parent = 0);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
//    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelInd*/ex &index) const;

protected:
    virtual bool eventFilter(QObject * editor, QEvent * event) override;

private slots:
    void on_lineEdit_textChanged(const QString &text);
    void commitAndCloseEditor();

private:
    CommandLineTokenizer* commandLineTokenizer;
    Option* gamsOption;
};

} // namespace studio
} // namespace gams

#endif // OPTIONCOMPLETERDELEGATE_H
