#ifndef GAMS_STUDIO_GDXDIFFDIALOG_FILEPATHLINEEDIT_H
#define GAMS_STUDIO_GDXDIFFDIALOG_FILEPATHLINEEDIT_H

#include <QLineEdit>
#include <QDragEnterEvent>

namespace gams {
namespace studio {
namespace gdxdiffdialog {

class FilePathLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    FilePathLineEdit(QWidget *parent=nullptr);

public slots:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;


};

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXDIFFDIALOG_FILEPATHLINEEDIT_H
