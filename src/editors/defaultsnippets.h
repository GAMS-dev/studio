/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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

#include <QString>
#include <QFile>

#include "logger.h"

inline void createDefaultSnippetFile(const QString &snippetFile)
{
    static const char *defaultJson = R"([
  {
    "trigger": "..",
    "description": "equation",
    "body": "${1:eq}.. ${2:stm1} =${3:e}= ${4:stm2};${0}"
  },
  {
    "trigger": "Set1",
    "description": "short",
    "body": "Set ${1:sym} /\n   ${2:e1},\n   ${3:e2} /;${0}"
  },
  {
    "trigger": "Set2",
    "description": "extended",
    "body": "Set ${1:sym} '${2:text}' /\n   ${3:e1} '${4:text1}',\n   ${5:e2} '${6:text2}' /;${0}"
  },
  {
    "trigger": "Alias",
    "description": "",
    "body": "Alias (${1:v1}, ${2:v2});${0}"
  },
  {
    "trigger": "Variable",
    "description": "",
    "body": "Variable\n   ${1:v1}(${2:i1})\n   ${3:v2}(${4:i2});${0}"
  },
  {
    "trigger": "Scalar",
    "description": "",
    "body": "Scalar\n   ${1:s1} ${2:text1} / ${3:value1} /\n   ${4:s2} ${5:text2} / ${6:value2} /;${0}"
  },
  {
    "trigger": "Parameter1",
    "description": "short",
    "body": "Parameter\n   ${1:v1}(${2:i1})\n   ${3:v2}(${4:i2});${0}"
  },
  {
    "trigger": "Parameter2",
    "description": "extended",
    "body": "Parameter ${1:p}(${2:s}) '${3:text}'\n   / ${4:e1} = ${5:v1},\n     ${6:e2} = ${7:v2} /;${0}"
  },
  {
    "trigger": "Equation",
    "description": "",
    "body": "Equation\n   ${1:eq1} '${2:text1}'\n   ${3:eq2} '${4:text2}';${0}"
  },
  {
    "trigger": "Function",
    "description": "",
    "body": "Function\n   ${1:fn1} /${2:def1}/\n   ${3:fn2} /${4:def2}/;${0}"
  },
  {
    "trigger": "Model",
    "description": "",
    "body": "Model\n   ${1:name1} '${2:text1}' / ${3:def1} /\n   ${4:name2} '${5:text2}' / ${6:def2} /;${0}"
  },
  {
    "trigger": "Table",
    "description": "",
    "body": "Table ${1:t1}(${2:s1},${3:s2}) '${4:text}'\n        ${5:s1h1} ${6:s1h2}\n   ${7:s2h1}  ${8:1x1}  ${9:1x2}\n   ${10:s2h2}  ${11:2x1}  ${12:2x2};"
  },
  {
    "trigger": "File",
    "description": "",
    "body": "File ${1:name} '${2:text}' /${3:file}/;${0}"
  },
  {
    "trigger": "for",
    "description": "loop",
    "body": "for (${1:i} = ${2:1} to ${3:10},\n   ${4:statement}\n);"
  },
  {
    "trigger": "loop",
    "description": "loop over a set",
    "body": "loop (${1:set},\n   ${2:statement}\n);"
  },
  {
    "trigger": "if",
    "description": "",
    "body": "if (${1:condition},\n   ${2:statement}\n);"
  },
  {
    "trigger": "while",
    "description": "loop",
    "body": "while (${1:condition},\n   ${2:statement}\n);"
  },
  {
    "trigger": "embeddedCode",
    "description": "",
    "body": "embeddedCode ${1:python}:\n   ${2:statement}\nendEmbeddedCode"
  },
  {
    "trigger": "embeddedCodeS",
    "description": "",
    "body": "embeddedCodeS ${1:python}:\n   ${2:statement}\nendEmbeddedCode"
  },
  {
    "trigger": "embeddedCodeV",
    "description": "",
    "body": "embeddedCodeV ${1:python}:\n   ${2:statement}\nendEmbeddedCode"
  },
  {
    "trigger": "$onEmbeddedCode",
    "description": "",
    "body": "$onEmbeddedCode ${1:python}:\n   ${2:statement}\n$offEmbeddedCode"
  },
  {
    "trigger": "$onEmbeddedCodeS",
    "description": "",
    "body": "$onEmbeddedCodeS ${1:python}:\n   ${2:statement}\n$offEmbeddedCode"
  },
  {
    "trigger": "$onEmbeddedCodeV",
    "description": "",
    "body": "$onEmbeddedCodeV ${1:python}:\n   ${2:statement}\n$offEmbeddedCode"
  },
  {
    "trigger": "$onEcho",
    "description": "",
    "body": "$onEcho > ${1:file}\n${2:output}\n$offEcho"
  },
  {
    "trigger": "$onEchoS",
    "description": "",
    "body": "$onEchoS > ${1:file}\n${2:output}\n$offEcho"
  },
  {
    "trigger": "$onEchoV",
    "description": "",
    "body": "$onEchoV > ${1:file}\n${2:output}\n$offEcho"
  },
  {
    "trigger": "$onPut",
    "description": "",
    "body": "$onPut\n${1:text}\n$offPut"
  },
  {
    "trigger": "$onPutS",
    "description": "",
    "body": "$onPutS\n${1:text}\n$offPut"
  },
  {
    "trigger": "$onPutV",
    "description": "",
    "body": "$onPutV\n${1:text}\n$offPut"
  },
  {
    "trigger": "$onExternalInput",
    "description": "",
    "body": "$onExternalInput\n${1:text}\n$offExternalInput"
  },
  {
    "trigger": "$onExternalOutput",
    "description": "",
    "body": "$onExternalOutput\n${1:text}\n$offExternalOutput"
  },
  {
    "trigger": "$ifThen",
    "description": "",
    "body": "$ifThen ${1:condition}\n   ${2:statement}\n$endIf"
  },
  {
    "trigger": "$ifThenI",
    "description": "",
    "body": "$ifThenI ${1:condition}\n   ${2:statement}\n$endIf"
  },
  {
    "trigger": "$ifThenE",
    "description": "",
    "body": "$ifThenE ${1:condition}\n   ${2:statement}\n$endIf"
  },
  {
    "trigger": "$onFold",
    "description": "",
    "body": "$onFold\n${1:code}\n$offFold"
  },
  {
    "trigger": "$onText",
    "description": "",
    "body": "$onText\n${1:comment}\n$offText"
  },
  {
    "trigger": "solve",
    "description": "",
    "body": "solve ${1:modelName} using ${2:lp} minimizing ${3:objVar};${0}"
  },
  {
    "trigger": "sum",
    "description": "",
    "body": "sum(${1:a}, ${2:b})${0}"
  },
  {
    "trigger": "prod",
    "description": "",
    "body": "prod(${1:a}, ${2:b})${0}"
  },
  {
    "trigger": "max",
    "description": "",
    "body": "max(${1:a}, ${2:b})${0}"
  },
  {
    "trigger": "min",
    "description": "",
    "body": "min(${1:a}, ${2:b})${0}"
  },
  {
    "trigger": "smax",
    "description": "",
    "body": "smax(${1:a}, ${2:b})${0}"
  },
  {
    "trigger": "smin",
    "description": "",
    "body": "smin(${1:a}, ${2:b})${0}"
  },
  {
    "trigger": "samd",
    "description": "",
    "body": "sand(${1:a}, ${2:b})${0}"
  },
  {
    "trigger": "sor",
    "description": "",
    "body": "sor(${1:a}, ${2:b})${0}"
  },
  {
    "trigger": "power",
    "description": "",
    "body": "power(${1:x}, ${2:y})${0}"
  }
])";

    // create default file if missing
    QFile f(snippetFile);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write(defaultJson);
        f.close();
        DEB() << "Default snippets written to " << snippetFile;
    } else
        DEB() << "Could not write snippets to " << snippetFile;
}
