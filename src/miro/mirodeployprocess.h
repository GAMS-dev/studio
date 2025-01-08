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
#ifndef MIRODEPLOYPROCESS_H
#define MIRODEPLOYPROCESS_H

#include "abstractmiroprocess.h"

namespace gams {
namespace studio {
namespace miro {

enum class MiroTargetEnvironment
{
    SingleUser      = 0,
    LocalMultiUser  = 1,
    MultiUser       = 2
};

class MiroDeployProcess final : public AbstractMiroProcess
{
    Q_OBJECT

public:
    MiroDeployProcess(QObject *parent = nullptr);

    QStringList defaultParameters() const override;

    void setBaseMode(bool baseMode);
    void setHypercubeMode(bool hcubeMode);
    void setTestDeployment(bool testDeploy);
    void setTargetEnvironment(MiroTargetEnvironment targetEnvironment);

protected slots:
    void completed(int exitCode) override;

protected:
    QProcessEnvironment miroProcessEnvironment() override;

private:
    void setupMiroPath(const QString &path);
    QString deployMode();

private:
    bool mBaseMode;
    bool mHypercubeMode;
    bool mTestDeployment;
    MiroTargetEnvironment mTargetEnvironment;
};

}
}
}

#endif // MIRODEPLOYPROCESS_H
