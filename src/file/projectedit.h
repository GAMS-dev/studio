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
#ifndef GAMS_STUDIO_PROJECTEDIT_H
#define GAMS_STUDIO_PROJECTEDIT_H

#include "abstractview.h"
#include <QFrame>
#include <QLineEdit>
#include "common.h"

class QComboBox;

namespace gams {
namespace studio {
class PExProjectNode;

namespace project {

namespace Ui {
class ProjectEdit;
}

class ProjectData : public QObject
{
    Q_OBJECT
public:
    enum Field {
        none     = 0x0000,
        file     = 0x0001,
        name     = 0x0002,
        nameExt  = 0x0004,
        workDir  = 0x0008,
        baseDir  = 0x0010,
        mainFile = 0x0020,
        pfFile   = 0x0040,
        hasGsp   = 0x0080,
        dynMain  = 0x0100,
        ownBase  = 0x0200,
        all      = 0xffff,
    };
    Q_DECLARE_FLAGS(Fields, Field)
    Q_FLAG(Fields)

    ProjectData(PExProjectNode *project);
    virtual ~ProjectData() override {}
    void setFieldData(Field field, const QString& value);
    QString fieldData(Field field);
    PExProjectNode *project() { return mProject; }
    bool isValidName(const QString &name);
    bool save();

signals:
    void changed(gams::studio::project::ProjectData::Field field);
    void tabNameChanged(gams::studio::PExProjectNode *project);
    void projectFilesChanged();

private slots:
    void projectChanged(const gams::studio::NodeId &id);

private:
    void updateFile(FileKind kind, const QString &path);

private:
    QHash<Field, QString> mData;
    QHash<Field, QString> mDataOri;
    PExProjectNode *mProject;
};


class ProjectEdit : public AbstractView
{
    Q_OBJECT

public:

    explicit ProjectEdit(ProjectData *sharedData, QWidget *parent = nullptr);
    ~ProjectEdit() override;
    ProjectData *sharedData() const;
    QString tabName(NameModifier mod = NameModifier::raw);
    bool isModified() const;
    bool save();
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void modificationChanged(bool modification);
    void saveProjects();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void updateData(ProjectData::Field field);
    void updateComboboxEntries();

    void on_edName_textChanged(const QString &text);
    void on_edWorkDir_textChanged(const QString &text);
    void on_edBaseDir_textChanged(const QString &text);
    void on_bGspSwitch_clicked();
    void on_bWorkDir_clicked();
    void on_bBaseDir_clicked();
    void on_cbMainFile_currentIndexChanged(int index);
    void on_cbDynamicMainFile_toggled(bool checked);
    void on_cbPfFile_currentIndexChanged(int index);

    void on_cbOwnBaseDir_toggled(bool checked);

private:
    void setSharedData(ProjectData *sharedData);
    bool isValidName(const QString &name);
    void updateEditColor(QLineEdit *edit, const QString &text);
    void updateState();
    void showDirDialog(const QString &title, QLineEdit *lineEdit, const QString &defaultDir);
    QStringList files(FileKind kind);
    void updateChanged(QComboBox *comboBox, const QStringList &data);
    void saveIfChanged();

    Ui::ProjectEdit *ui;
    ProjectData *mSharedData;
    bool mInitDone = false;
    bool mModified = false;
    bool mBlockUpdate = false;
    QString mName;
};


} // namespace project
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_PROJECTEDIT_H
