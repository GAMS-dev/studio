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
#ifndef SOLVEROPTIONWIDGET_H
#define SOLVEROPTIONWIDGET_H

#include <QMenu>
#include <QWidget>

#include "common.h"
#include "solveroptiontablemodel.h"

namespace gams {
namespace studio {

class MainWindow;

namespace option {

namespace Ui {
class SolverOptionWidget;
}

class OptionTokenizer;

class SolverOptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SolverOptionWidget(QString solverName, QString optionFilePath, FileId id, QTextCodec* mCodec, QWidget *parent = nullptr);
    ~SolverOptionWidget();

    bool isInFocused(QWidget* focusWidget);

    FileId fileId() const;

    bool isModified() const;
    void setModified(bool modified);

    bool saveAs(const QString &location);

    bool isAnOptionWidgetFocused(QWidget* focusWidget) const;
    QString getSelectedOptionName(QWidget* widget) const;

    QString getSolverName() const;
    int getItemCount() const;

    bool isViewCompact() const;

    void selectSearchField() const;

signals:
    void modificationChanged(bool modifiedState);
    void itemCountChanged(int newItemCount);
    void compactViewChanged(bool compact);

public slots:
    void showOptionContextMenu(const QPoint &pos);
    void showDefinitionContextMenu(const QPoint &pos);
    void addOptionFromDefinition(const QModelIndex &index);

    void on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void on_newTableRowDropped(const QModelIndex &index);

    bool saveOptionFile(const QString &location);

    void on_reloadSolverOptionFile(QTextCodec* codec);
    void on_selectRow(int logicalIndex);
    void on_toggleRowHeader(int logicalIndex);

    void on_compactViewCheckBox_stateChanged(int checkState);
    void on_saveButton_clicked(bool checked = false);
    void on_saveAsButton_clicked(bool checked = false);
    void on_openAsTextButton_clicked(bool checked = false);

   void on_addCommentAboveChanged(int checkState);
   void on_addEOLCommentChanged(int checkState);

private slots:
    void showOptionDefinition();
    void copyDefinitionToClipboard(int column);

    void toggleCommentOption();
    void selectAnOption();
    void insertOption();
    void insertComment();
    void deleteOption();

private:
    Ui::SolverOptionWidget *ui;
    FileId mFileId;
    QString mLocation;
    QString mSolverName;

    bool addCommentAbove = false;
    bool addEOLComment = false;

    QTextCodec* mCodec;
    SolverOptionTableModel* mOptionTableModel;
    QMenu mContextMenu;

    bool mModified;
    OptionTokenizer* mOptionTokenizer;

    void addActions();
    void updateEditActions(bool modified);
    void updateTableColumnSpan();

    MainWindow* getMainWindow();

};


}
}
}
#endif // SOLVEROPTIONWIDGET_H
