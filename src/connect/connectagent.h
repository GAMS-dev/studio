/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#ifndef CONNECTAGENT_H
#define CONNECTAGENT_H

#include <QString>
#include "yaml-cpp/yaml.h"

namespace gams {
namespace studio {
namespace connect {

class ConnectAgent
{
protected:
    ConnectAgent();
    ConnectAgent(const YAML::Node& node);
    virtual void loadFromFile(const QString& inputFileName);
    virtual void loadFromString(const QString& input);
    void unload(const QString& outputFileName);

    YAML::Node mRootNode;

public:
    YAML::Node getRootNode() { return mRootNode; }
    const std::string str() {
        YAML::Emitter emitter;
        emitter << mRootNode;
        return std::string(emitter.c_str());
    }

};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // CONNECTAGENT_H
