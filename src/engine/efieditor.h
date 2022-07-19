#ifndef GAMS_STUDIO_EFI_EFIEDITOR_H
#define GAMS_STUDIO_EFI_EFIEDITOR_H

#include "abstractview.h"

namespace gams {
namespace studio {
namespace efi {

namespace Ui {
class EfiEditor;
}

class EfiEditor : public AbstractView
{
    Q_OBJECT
public:
    explicit EfiEditor(QWidget *parent = nullptr);
    ~EfiEditor() override;
    void setWorkingDir(const QString &workDir);
    void load(const QString &fileName);
    void save(const QString &fileName);
    bool isModified();

signals:
    void modificationChanged(bool modiState);
    void requestSave();

private slots:

private:
    void updateInfoText(QString extraText, bool valid);
    void setModified(bool modified);

private:
    Ui::EfiEditor *ui;
    QString mFileName;
    bool mModified = false;
};


} // namespace efi
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_EFI_EFIEDITOR_H
