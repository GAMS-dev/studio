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
#include "findworker.h"
#include <QRegularExpression>

namespace gams {
namespace studio {
namespace find {

FindWorker::FindWorker(QObject *parent) : QObject(parent)
{
}

void FindWorker::findText(const QString &txt, const QRegularExpression &rex, int startPos, bool backward, int id)
{
    FindResult res;
    res.id = id;
    if (txt.isEmpty() || !rex.isValid()) {
        emit done(res);
        return;
    }
    QList<int> positions;
    QList<int> lengths;
    auto it = rex.globalMatch(txt);

    while (it.hasNext()) {
        if (activeFindId.load() != id) {
            emit done({-1, 0, id, false, false, true});
            return;
        }
        QRegularExpressionMatch m = it.next();
        positions.append(m.capturedStart());
        lengths.append(m.capturedLength());
    }
    if (positions.isEmpty()) {
        emit done(res);
        return;
    }
    int target = -1;
    if (backward) {
        for (int i = positions.size() - 1; i >= 0; --i) {
            if (positions[i] <= startPos) {
                target = i;
                break;
            }
        }
        if (target == -1) {
            target = positions.size() - 1;
            res.wrapped = true;
        }
    } else {
        for (int i = 0; i < positions.size(); ++i) {
            if (positions[i] >= startPos) {
                target = i;
                break;
            }
        }
        if (target == -1) {
            target = 0;
            res.wrapped = true;
        }
    }

    if (target != -1) {
        res.success = true;
        res.pos = positions[target];
        res.len = lengths[target];
    }
    emit done(res);
}

} // namespace find
} // namespace studio
} // namespace gams
