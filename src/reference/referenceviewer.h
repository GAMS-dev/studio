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

#ifndef REFERENCEVIEWER_H
#define REFERENCEVIEWER_H

#include <QWidget>
#include <QList>
#include <QMap>
#include <QTabWidget>
#include <QProcess>

#include "reference.h"
#include "abstractview.h"

namespace Ui {
class ReferenceViewer;
}
class QStandardItemModel;

namespace gams {
namespace studio {
namespace reference {

enum class ReferenceViewerType {
    undefined = 0,
    Symbol,
    FileUsed
};

class Reference;

class ReferenceViewer : public AbstractView
{
    Q_OBJECT

public:
    explicit ReferenceViewer(const QString &referenceFile, const QString &encodingName, QWidget *parent = nullptr);
    ~ReferenceViewer() override;
    void selectSearchField() const;
    void updateStyle();
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void jumpTo(gams::studio::reference::ReferenceItem item);
    void referenceTo(gams::studio::reference::ReferenceItem item);
    void processState(QProcess::ProcessState &state);

public slots:
    void reloadFile(const QString &encodingName);
    void updateView(bool loadStatus, bool pendingReload);
    void updateFileUsedTabText(bool compactView);
    int currentSelectedTab();
    gams::studio::reference::ReferenceSettings saveSettings();
    void loadSettings(const gams::studio::reference::ReferenceSettings &settings);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void setCurrentViewerIndex(int index);
    void selectStackedIndex(int index);
    void updateTabs();

private:
    Ui::ReferenceViewer *ui;
    QString mEncodingName;
    QScopedPointer<Reference> mReference;
    QModelIndex lastHoverIndex;
    QStandardItemModel *mNavModel = nullptr;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // REFERENCEVIEWER_H
