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
#include <fstream>
#include "connectdata.h"

namespace gams {
namespace studio {
namespace connect {

ConnectData::ConnectData(const YAML::Node& node)
{
    mRootNode = node;
}

ConnectData::ConnectData(const QString &inputFileName)
{
    loadFromFile(inputFileName);
}

ConnectData::~ConnectData()
{

}

void ConnectData::loadFromFile(const QString &inputFileName)
{
    ConnectAgent::loadFromFile(inputFileName);
}

void ConnectData::loadFromString(const QString &input)
{
    ConnectAgent::loadFromString(input);
}

void ConnectData::unload(const QString &outputFileName)
{
    YAML::Emitter emitter;
    emitter.SetNullFormat(YAML::EMITTER_MANIP::LowerNull);
    format(emitter, mRootNode);
    emitter << YAML::Newline;

    std::ofstream fout(outputFileName.toStdString());
    fout << emitter.c_str();
}

void ConnectData::format(YAML::Emitter& emitter, const YAML::Node& node) {
    if (node.Type()==YAML::NodeType::Map) {
        emitter << YAML::BeginMap;
        for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
            emitter << YAML::Key << it->first.as<std::string>();
            format(emitter, it->second);
        }
        emitter << YAML::EndMap;
    } else if (node.Type()==YAML::NodeType::Sequence) {
               emitter << YAML::BeginSeq;
               for (size_t i=0; i<node.size(); ++i) {
                   format(emitter, node[i]);
               }
               emitter << YAML::EndSeq;
    } else if (node.Type()==YAML::NodeType::Scalar) {
              QString str = QString::fromStdString(node.as<std::string>());
              if (str.contains("\n"))
                 emitter << YAML::Literal;
              emitter << node.as<std::string>();
    } else /*if (node.Type()==YAML::NodeType::Null)*/ {
               emitter << node;
    }
}

}// namespace connect
} // namespace studio
} // namespace gams
