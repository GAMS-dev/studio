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
#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include "abstractview.h"

namespace Ui {
class WelcomePage;
}

class QLabel;

namespace gams {
namespace studio {

struct HistoryData;
class MainWindow;

class WelcomePage : public AbstractView
{
    Q_OBJECT
public:
    explicit WelcomePage(MainWindow *parent = nullptr);
    ~WelcomePage() override;
    void zoomReset();
    void setDocEnabled(bool enabled);

signals:
    void openFilePath(const QString &filePath);
    void openProject(const QString &projectPath);
    void removeFromHistory(const QString &path);
    void relayActionWp(const QString &action);
    void relayModLibLoad(const QString &lib, bool forceOverwrite = false);
    void relayDocOpen(const QString &doc, const QString &anchor);

public slots:
    void historyChanged();
    void on_relayAction(const QString &action);
    void on_relayModLibLoad(const QString &lib);
    void on_relayOpenDoc(const QString &doc, const QString &anchor);

private slots:
    void handleZoom(int delta);

protected:
    bool event(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void setupIcons();

private:
    Ui::WelcomePage *ui;
    MainWindow *mMain;
    bool mOutputVisible;
};

}
}

#endif // WELCOMEPAGE_H
