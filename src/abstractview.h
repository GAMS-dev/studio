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
#ifndef GAMS_STUDIO_ABSTRACTVIEW_H
#define GAMS_STUDIO_ABSTRACTVIEW_H

#include <QWidget>
#include <QHeaderView>

#include "common.h"

namespace gams {
namespace studio {

class AbstractView: public QWidget
{
    typedef QList<qreal> ColumnWidths;
    typedef QHash<QHeaderView*, ColumnWidths> HeadersData;
    Q_OBJECT
public:
    AbstractView(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~AbstractView() override;
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);

signals:
    void zoomRequest(int delta);

protected slots:
    void headerRegister(QHeaderView *header);
    void headerUpdateAll();

protected:
    void headerStore(QHeaderView *header, int logicalIndex, int oldSize, int newSize);
    void headerResetAll();
    qreal headerCurrentScale() const;
    void wheelEvent(QWheelEvent *e) override;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void zoomInF(qreal range);

private slots:
    void headerUnregister(QObject *object);

private:
    void headerUpdate(HeadersData::iterator &it);

private:
    HeadersData mHeaders;
    qreal mBaseScale = -1.0;
    bool mInternalResize = false;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_ABSTRACTVIEW_H
