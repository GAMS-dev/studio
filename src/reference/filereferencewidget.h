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
#ifndef FILEREFERENCEWIDGET_H
#define FILEREFERENCEWIDGET_H

#include <QWidget>

#include "reference.h"
#include "referenceviewer.h"
#include "fileusedtreemodel.h"

namespace Ui {
class FileReferenceWidget;
}

namespace gams {
namespace studio {
namespace reference {

class ReferenceViewer;

class FileReferenceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileReferenceWidget(Reference* ref, ReferenceViewer *parent = nullptr);
    ~FileReferenceWidget() override;
    QList<QHeaderView*> headers();
    bool isModelLoaded() const;

    bool isViewCompact();

    void activateFilter();
    void deActivateFilter();

public slots:
    void resetModel();
    void initModel();
    void initModel(gams::studio::reference::Reference *ref);

    void expandResetModel();
    void jumpToFile(const QModelIndex &index);
    void jumpToReferenceItem(const QModelIndex &index);

private:
    Ui::FileReferenceWidget *ui;
    FileUsedTreeModel* mFileUsedModel;

    bool mFilterActivated;
    Reference* mReference;
    ReferenceViewer* mReferenceViewer;

};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // FILEREFERENCEWIDGET_H
