#ifndef GAMS_STUDIO_EFI_EFIEDITOR_H
#define GAMS_STUDIO_EFI_EFIEDITOR_H

#include <QWidget>

namespace gams {
namespace studio {
namespace efi {

namespace Ui {
class EfiEditor;
}

class EfiEditor : public QWidget
{
    Q_OBJECT

public:
    explicit EfiEditor(QWidget *parent = nullptr);
    ~EfiEditor() override;
    void setWorkingDir(const QString &workDir);
    void load(const QString &fileName);

private slots:
    void writeFile();

private:
    Ui::EfiEditor *ui;
    QString mFileName;
};


} // namespace efi
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_EFI_EFIEDITOR_H
