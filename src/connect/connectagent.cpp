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
#include <fstream>
#include "connectagent.h"
#include "exception.h"

namespace gams {
namespace studio {
namespace connect {

void ConnectAgent::loadFromFile(const QString &inputFileName)
{
    try {
        mRootNode = YAML::LoadFile(inputFileName.toStdString());
    } catch(const YAML::ParserException& e) {
        EXCEPT() << "Error : " << e.what() << " : " << inputFileName;
    }
}

void ConnectAgent::loadFromString(const QString &input)
{
    try {
        mRootNode = YAML::Load(input.toStdString());
    } catch(const YAML::ParserException& e) {
        EXCEPT() << "Error : " << e.what() << " : when loading from string : " << input;
    }

}

} // namespace connect
} // namespace studio
} // namespace gams
