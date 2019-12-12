#ifndef GDXDIFFPROCESS_H
#define GDXDIFFPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {
namespace gdxdiffdialog {

class GdxDiffProcess
        : public AbstractGamsProcess
{
    Q_OBJECT

public:
    GdxDiffProcess(QObject *parent = Q_NULLPTR);

    void execute() override;
    void setInput1(const QString &input1);
    void setInput2(const QString &input2);
    void setDiff(const QString &diff);
    void setIgnoreSetText(bool ignoreSetText);
    void setDiffOnly(bool diffOnly);
    void setFieldOnly(bool fieldOnly);
    void setFieldToCompare(const QString &fieldToCompare);
    void setEps(const QString &eps);
    void setRelEps(const QString &relEps);
    QString diffFile() const;
    void stop(int waitMSec=0);

private slots:
    void appendSystemLog(const QString &text);

private:
    QString mInput1;
    QString mInput2;
    QString mDiff;
    QString mEps;
    QString mRelEps;
    bool mIgnoreSetText;
    bool mDiffOnly;
    bool mFieldOnly;
    QString mFieldToCompare;
    QString mDiffFile;
};

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams

#endif // GDXDIFFPROCESS_H
