/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef CONNECTDATA_H
#define CONNECTDATA_H

#include <QString>
#include "connectagent.h"

namespace gams {
namespace studio {
namespace connect {

class ConnectData : public ConnectAgent
{
public:
    ConnectData();
    ConnectData(const YAML::Node& node);
    ConnectData(const QString& inputFileName);
    ~ConnectData() override;

    void unload(const QString& outputFileName);
protected:
    void format(YAML::Emitter& emitter, const YAML::Node& node);
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // CONNECTDATA_H
