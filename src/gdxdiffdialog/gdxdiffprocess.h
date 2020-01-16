#ifndef GDXDIFFPROCESS_H
#define GDXDIFFPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {
namespace gdxdiffdialog {

class GdxDiffProcess final
        : public AbstractGamsProcess
{
    Q_OBJECT

public:
    GdxDiffProcess(QObject *parent = nullptr);

    void execute() override;

    QString diffFile() const;
    void stop(int waitMSec=0);

private slots:
    void appendSystemLog(const QString &text);

private:
    QString mDiffFile;
};

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams

#endif // GDXDIFFPROCESS_H
