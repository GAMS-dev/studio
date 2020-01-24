/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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

#include <QWidget>

#include "common.h"

namespace Ui {
class WelcomePage;
}

class QLabel;

namespace gams {
namespace studio {

struct HistoryData;
class MainWindow;

class WelcomePage : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePage(MainWindow *parent = nullptr);
    ~WelcomePage();

signals:
    void openFilePath(const QString &filePath, bool focus = true, int codecMib = -1, bool forcedAsTextEdit = false);
    void relayActionWp(QString action);
    void relayModLibLoad(QString lib, bool forceOverwrite = false);
    void relayDocOpen(QString doc, QString anchor);

public slots:
    void historyChanged();
    void on_relayAction(QString action);
    void on_relayModLibLoad(QString lib);
    void on_relayOpenDoc(QString doc, QString anchor);

private slots:
    void linkActivated(const QString &link);

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private:
    Ui::WelcomePage *ui;
    QList<QLabel*> mFileHistory;
    MainWindow *mMain;
    bool mOutputVisible;
};

}
}

#endif // WELCOMEPAGE_H
