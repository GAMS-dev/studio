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
#ifndef CONNECTEDITOR_H
#define CONNECTEDITOR_H

#include "abstractview.h"
#include "connect.h"
#include "connectdatamodel.h"

namespace gams {
namespace studio {

class MainWindow;

namespace connect {

namespace Ui {
class ConnectEditor;
}

class Connect;

class ConnectEditor : public AbstractView
{
    Q_OBJECT

public:
    explicit ConnectEditor(const QString& connectDataFileName, const FileId &id,  QWidget *parent = nullptr);
    ~ConnectEditor() override;

    FileId fileId() const;

    bool saveAs(const QString &location);

    bool isModified() const;
    void setModified(bool modified);

    QString getSelectedAgentName() const;
    QString getSelectedAttributeName() const;

signals:
    void modificationChanged(bool modifiedState);

public slots:
    bool saveConnectFile(const QString &location);
    void openAsTextButton_clicked(bool checked = false);
    void on_reloadConnectFile();

private slots:
    void fromSchemaInserted(const QString& schemaname, int position);
    void schemaDoubleClicked(const QModelIndex &modelIndex);
    void expandAndResizedToContents(const QModelIndex &index);

    void schemaHelpRequested(const QString &schemaName);
    void appendItemRequested(const QModelIndex &index);
    void appendSchemaItemRequested(const int schemaNumber, const QModelIndex &index);
    void deleteDataItemRequested(const QModelIndex &index);
    void moveUpDatatItemRequested(const QModelIndex &index);
    void moveDownDatatItemRequested(const QModelIndex &index);

    bool checkProblemLoad();

private:
    bool init(bool quiet=false);

    void saveExpandedState();
    void restoreExpandedState();

    void saveExpandedOnLevel(const QModelIndex& index);
    void restoreExpandedOnLevel(const QModelIndex& index);

private:
    static QRegularExpression mOneOfPattern;

    Ui::ConnectEditor *ui;
    bool        mModified;
    FileId      mFileId;

    QList<int>        mExpandIDs;
    ConnectDataModel* mDataModel;
    Connect*          mConnect;
    QString           mLocation;

    MainWindow*       getMainWindow();
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // CONNECTEDITOR_H
