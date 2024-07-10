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
#ifndef HELPVIEW_H
#define HELPVIEW_H

#include <QWebEngineView>

namespace gams {
namespace studio {
namespace help {

class HelpPage;

class HelpView : public QWebEngineView
{
   Q_OBJECT

public:
    HelpView(QWidget *parent = nullptr);
    void setPage(QWebEnginePage *page);
    void setCurrentHoveredLink(const QString &url);

protected:
    bool event(QEvent *e) override;
    bool eventFilter(QObject * obj, QEvent * e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;

private:
    QString mCurrentHovered;
};

} // namespace help
} // namespace studio
} // namespace gams

#endif // HELPVIEW_H
