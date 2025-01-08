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
#ifndef GAMSMIROPROCESS_H
#define GAMSMIROPROCESS_H

#include "abstractmiroprocess.h"

namespace gams {
namespace studio {
namespace miro {

enum class MiroMode
{
    Base,
    Configuration
};

class MiroProcess final : public AbstractMiroProcess
{
    Q_OBJECT

public:
    MiroProcess(QObject *parent = nullptr);

    void execute() override;

    QStringList defaultParameters() const override;

    void setMiroMode(MiroMode mode);

    void setSkipModelExecution(bool skipModelExeution) {
        mSkipModelExecution = skipModelExeution;
    }

protected:
    QProcessEnvironment miroProcessEnvironment() override;

private:
    void setupMiroEnvironment();

private:
    MiroMode mMiroMode;
    bool mSkipModelExecution;
};

}
}
}

#endif // GAMSMIROPROCESS_H
