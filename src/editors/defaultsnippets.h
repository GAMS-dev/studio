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

static const char *defaultJson = R"([
  {
    "trigger": "..",
    "description": "equation",
    "body": "${1:eq}.. ${2:stm1} =${3:e}= ${4:stm2};${0}"
  },
  {
    "trigger": "Set1",
    "description": "short",
    "body": "Set
   ${1:s1}(${2:s1})
   ${3:s2}(${4:s1},${5:s2});${0}"
  },
  {
    "trigger": "Set2",
    "description": "extended",
    "body": "Set
   ${1:s1}        '${2:text-s1}' / ${3:[1]} 'text-[1]',
                         ${4:[2]} 'text-[2]',
                         ${5:[3]} 'text-[3]' /
   ${6:s2}        '${7:text-s2}' / ${8:s2h1} * ${9:s2h9} /
   ${10:s3}(${11:s1})    '${12:text-s3}' / ${13:[1]}, ${14:[3]} /
   ${15:s2}(${16:s2},${17:s3}) '${18:text-s4}' / ${19:s2h3}.${20:[1]} 'text-s2h3.[1]',
                         ${21:s2h5}.${22:[3]} 'text-s2h5.[3]' /;${0}"
  },
  {
    "trigger": "Singleton Set1",
    "description": "short",
    "body": "Singleton Set
   ${1:ss1}(${2:s1})
   ${3:ss2}(${4:s1},${5:s2});${0}"
  },
  {
    "trigger": "Singleton Set2",
    "description": "extended",
    "body": "Singleton Set
   ${1:ss1}        '${2:text-ss1}' / ${3:s1} 'text-s1' /
   ${4:ss2}(${5:s1})    '${6:text-ss2}' / ${7:[1]} 'text-[1]' /
   ${8:ss3}(${9:s1},${10:s2}) '${11:text-ss3}' / ${12:[1]}.${13:s2h4} 'text-[1].s2h4' /;${0}"
  },
  {
    "trigger": "Alias",
    "description": "",
    "body": "Alias (${1:v1}, ${2:v2});${0}"
  },
  {
    "trigger": "Variable1",
    "description": "short",
    "body": "Variable
   ${1:v1}
   ${2:v2}(${3:s1})
   ${4:v3}(${5:s1},${6:s2});${0}"
  },
  {
    "trigger": "Variable2",
    "description": "extended",
    "body": "Variable
   ${1:v1}        '${2:text-v1}' / ${3:l} 0.1 /
   ${4:v2}(${5:s1})    '${6:text-v2}' / ${7:[1].lo} 21,
                         ${8:[2].up} 22 /
   ${9:v3}(${10:s1},${11:s2}) '${12:text-v3}' / ${13:[1]}.${14:s2h1.up} 1,
                         ${15:[1]}.${16:s2h3.lo} 0,
                         ${17:[1]}.${18:s2h1.l} 0.2 /;${0}"
  },
  {
    "trigger": "Binary Variable1",
    "description": "short",
    "body": "Binary Variable
   ${1:v1}
   ${2:v2}(${3:s1})
   ${4:v3}(${5:s1},${6:s2});${0}"
  },
  {
    "trigger": "Binary Variable2",
    "description": "extended",
    "body": "Binary Variable
   ${1:v1}        '${2:text-v1}' / ${3:l} 1 /
   ${4:v2}(${5:s1})    '${6:text-v2}' / ${7:[1].lo} 0,
                         ${8:[2].up} 1 /
   ${9:v3}(${10:s1},${11:s2}) '${12:text-v3}' / ${13:[1]}.${14:s2h1.up} 1,
                         ${15:[1]}.${16:s2h3.lo} 0,
                         ${17:[1]}.${18:s2h1.l} 1 /;${0}"
  },
  {
    "trigger": "Integer Variable1",
    "description": "short",
    "body": "Integer Variable
   ${1:v1}
   ${2:v2}(${3:s1})
   ${4:v3}(${5:s1},${6:s2});${0}"
  },
  {
    "trigger": "Integer Variable2",
    "description": "extended",
    "body": "Integer Variable
   ${1:v1}        '${2:text-v1}' / ${3:l} 3 /
   ${4:v2}(${5:s1})    '${6:text-v2}' / ${7:[1].lo} 21,
                         ${8:[2].up} 22 /
   ${9:v3}(${10:s1},${11:s2}) '${12:text-v3}' / ${13:[1]}.${14:s2h1.up} 3,
                         ${15:[1]}.${16:s2h3.lo} 0,
                         ${17:[1]}.${18:s2h1.l} 2 /;${0}"
  },
  {
    "trigger": "Negative Variable1",
    "description": "short",
    "body": "Negative Variable
   ${1:v1}
   ${2:v2}(${3:s1})
   ${4:v3}(${5:s1},${6:s2});${0}"
  },
  {
    "trigger": "Negative Variable2",
    "description": "extended",
    "body": "Negative Variable
   ${1:v1}        '${2:text-v1}' / ${3:l} -0.1 /
   ${4:v2}(${5:s1})    '${6:text-v2}' / ${7:[1].lo} -22,
                         ${8:[2].up} -21 /
   ${9:v3}(${10:s1},${11:s2}) '${12:text-v3}' / ${13:[1]}.${14:s2h1.up} 0,
                         ${15:[1]}.${16:s2h3.lo} -1,
                         ${17:[1]}.${18:s2h1.l} -0.2 /;${0}"
  },
  {
    "trigger": "NonNegative Variable1",
    "description": "short",
    "body": "NonNegative Variable
   ${1:v1}
   ${2:v2}(${3:s1})
   ${4:v3}(${5:s1},${6:s2});${0}"
  },
  {
    "trigger": "NonNegative Variable2",
    "description": "extended",
    "body": "NonNegative Variable
   ${1:v1}        '${2:text-v1}' / ${3:l} 0.1 /
   ${4:v2}(${5:s1})    '${6:text-v2}' / ${7:[1].lo} 21,
                         ${8:[2].up} 22 /
   ${9:v3}(${10:s1},${11:s2}) '${12:text-v3}' / ${13:[1]}.${14:s2h1.up} 1,
                         ${15:[1]}.${16:s2h3.lo} 0,
                         ${17:[1]}.${18:s2h1.l} 0.2 /;${0}"
  },
  {
    "trigger": "Positive Variable1",
    "description": "short",
    "body": "Positive Variable
   ${1:v1}
   ${2:v2}(${3:s1})
   ${4:v3}(${5:s1},${6:s2});${0}"
  },
  {
    "trigger": "Positive Variable2",
    "description": "extended",
    "body": "Positive Variable
   ${1:v1}        '${2:text-v1}' / ${3:l} 0.1 /
   ${4:v2}(${5:s1})    '${6:text-v2}' / ${7:[1].lo} 21,
                         ${8:[2].up} 22 /
   ${9:v3}(${10:s1},${11:s2}) '${12:text-v3}' / ${13:[1]}.${14:s2h1.up} 1,
                         ${15:[1]}.${16:s2h3.lo} 0,
                         ${17:[1]}.${18:s2h1.l} 0.2 /;${0}"
  },
  {
    "trigger": "SemiCont Variable1",
    "description": "short",
    "body": "SemiCont Variable
   ${1:v1}
   ${2:v2}(${3:s1})
   ${4:v3}(${5:s1},${6:s2});${0}"
  },
  {
    "trigger": "SemiCont Variable2",
    "description": "extended",
    "body": "SemiCont Variable
   ${1:v1}        '${2:text-v1}' / ${3:l} 0.1 /
   ${4:v2}(${5:s1})    '${6:text-v2}' / ${7:[1].lo} 21,
                         ${8:[2].up} 22 /
   ${9:v3}(${10:s1},${11:s2}) '${12:text-v3}' / ${13:[1]}.${14:s2h1.up} 1,
                         ${15:[1]}.${16:s2h3.lo} 0,
                         ${17:[1]}.${18:s2h1.l} 0.2 /;${0}"
  },
  {
    "trigger": "SemiInt Variable1",
    "description": "short",
    "body": "SemiInt Variable
   ${1:v1}
   ${2:v2}(${3:s1})
   ${4:v3}(${5:s1},${6:s2});${0}"
  },
  {
    "trigger": "SemiInt Variable2",
    "description": "extended",
    "body": "SemiInt Variable
   ${1:v1}        '${2:text-v1}' / ${3:l} 1 /
   ${4:v2}(${5:s1})    '${6:text-v2}' / ${7:[1].lo} 21,
                         ${8:[2].up} 22 /
   ${9:v3}(${10:s1},${11:s2}) '${12:text-v3}' / ${13:[1]}.${14:s2h1.up} 3,
                         ${15:[1]}.${16:s2h3.lo} 0,
                         ${17:[1]}.${18:s2h1.l} 2 /;${0}"
  },
  {
    "trigger": "SOS1 Variable1",
    "description": "short",
    "body": "SOS1 Variable
   ${1:v1}(${2:s1})
   ${3:v2}(${4:s1},${5:s2});${0}"
  },
  {
    "trigger": "SOS1 Variable2",
    "description": "extended",
    "body": "SOS1 Variable
   ${1:v1}        '${2:text-v1}' / ${3:l} 0.1 /
   ${4:v2}(${5:s1})    '${6:text-v2}' / ${7:[1].lo} 0,
                         ${8:[2].up} 2 /
   ${9:v3}(${10:s1},${11:s2}) '${12:text-v3}' / ${13:[1]}.${14:s2h1.up} 2,
                         ${15:[1]}.${16:s2h3.lo} 0,
                         ${17:[1]}.${18:s2h1.l} 0.2 /;${0}"
  },
  {
    "trigger": "SOS2 Variable1",
    "description": "short",
    "body": "SOS2 Variable
   ${2:v1}(${3:s1})
   ${4:v2}(${5:s1},${6:s2});${0}"
  },
  {
    "trigger": "SOS2 Variable2",
    "description": "extended",
    "body": "SOS2 Variable
   ${1:v1}        '${2:text-v1}' / ${3:l} 0.1 /
   ${4:v2}(${5:s1})    '${6:text-v2}' / ${7:[1].lo} 0,
                         ${8:[2].up} 2 /
   ${9:v3}(${10:s1},${11:s2}) '${12:text-v3}' / ${13:[1]}.${14:s2h5.l} 2,
                         ${15:[1]}.${16:s2h6.l} 5,
                         ${17:[1]}.${18:s2h5.lo} 0.2 /;${0}"
  },
  {
    "trigger": "Scalar1",
    "description": "short",
    "body": "Scalar\n   ${1:s1}\n   ${3:s2};${0}"
  },
  {
    "trigger": "Scalar2",
    "description": "extended",
    "body": "Scalar
   ${1:s1} '${2:text-s1}' / ${3:1} /
   ${4:s2} '${5:text-s2}' / ${6:2} /;${0}"
  },
  {
    "trigger": "Parameter1",
    "description": "short",
    "body": "Parameter
   ${1:p1}
   ${2:p2}(${3:s1})
   ${4:p3}(${5:s1},${6:s2});${0}"
  },
  {
    "trigger": "Parameter2",
    "description": "extended",
    "body": "Parameter
   ${1:p1}        '${2:text-p1}' / 123 /
   ${3:p2}(${4:s1})    '${5:text-p2}' / ${6:[1]} 1,
                         ${7:[2]} 2 /
   ${8:p3}(${9:s1},${10:s2}) '${11:text-p3}' / ${12:[1].s2h1} 1,
                         ${13:[2].s2h1} 2 /;${0}"
  },
  {
    "trigger": "Equation1",
    "description": "short",
    "body": "Equation\n   ${1:eq1}\n   ${3:eq2}(${4:s1});${0}"
  },
  {
    "trigger": "Equation2",
    "description": "extended",
    "body": "Equation
   ${1:eq1}        '${2:text-eq1}' / l 1 /
   ${3:eq2}(${4:s1})    '${5:text-eq2}' / ${6:[1].lo} 1,
                           ${7:[1].up} 2 /
   ${8:eq3}(${9:s1},${10:s2}) '${11:text-eq3}' / ${12:[1].s2h1.lo} 1,
                           ${13:[2].s2h1.up} 2 /;${0}"
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
    "trigger": "Table1",
    "description": "dim2",
    "body": "Table ${1:t1}(${2:s1},${3:s2}) '${4:text}'
   [0]  ${5:s2h1} ${6:s2h2} ${7:s2h3}
   ${8:[1]_}   ${9:1.1}  ${10:1.2}  ${11:1.3}
   ${12:[2]_}   ${13:2.1}  ${14:2.2}  ${15:2.3};${0}"
  },
  {
    "trigger": "Table2",
    "description": "dim4",
    "body": "Table ${1:t1}(${2:i},${3:j},${4:k},${5:l}) '${6:text}'
          ${7:k1}.${8:l1} ${9:k1}.${10:l2} ${11:k2}.${12:l1}
   ${13:i1}.${14:j1}    ${15:1.1}   ${16:1.2}   ${17:1.3}
   ${18:i1}.${19:j2}    ${20:2.1}   ${21:2.2}   ${22:2.3};${0}"
  },
  {
    "trigger": "File",
    "description": "",
    "body": "File ${1:name} '${2:text}' /${3:file}/;${0}"
  },
  {
    "trigger": "if",
    "description": "",
    "body": "if (${1:condition},\n   ${2:statement};\n);"
  },
  {
    "trigger": "for",
    "description": "loop",
    "body": "for (${1:i} = ${2:1} to ${3:10},\n   ${4:statement};${0}\n);"
  },
  {
    "trigger": "loop",
    "description": "loop over a set",
    "body": "loop (${1:s1},\n   ${2:statement};${0}\n);"
  },
  {
    "trigger": "while",
    "description": "loop",
    "body": "while (${1:condition},\n   ${2:statement};${0}\n);"
  },
  {
    "trigger": "repeat",
    "description": "loop",
    "body": "repeat (\n   ${2:statement};${0}\nuntil ${1:condition} );"
  },
  {
    "trigger": "embeddedCode",
    "description": "",
    "body": "embeddedCode ${1:python}:\n   ${2:statement}${0}\nendEmbeddedCode"
  },
  {
    "trigger": "embeddedCodeS",
    "description": "",
    "body": "embeddedCodeS ${1:python}:\n   ${2:statement}${0}\nendEmbeddedCode"
  },
  {
    "trigger": "embeddedCodeV",
    "description": "",
    "body": "embeddedCodeV ${1:python}:\n   ${2:statement}${0}\nendEmbeddedCode"
  },
  {
    "trigger": "continueEmbeddedCode",
    "description": "",
    "body": "continueEmbeddedCode ${1:python}:\n   ${2:statement}${0}\nendEmbeddedCode"
  },
  {
    "trigger": "continueEmbeddedCodeS",
    "description": "",
    "body": "continueEmbeddedCodeS ${1:python}:\n   ${2:statement}${0}\nendEmbeddedCode"
  },
  {
    "trigger": "continueEmbeddedCodeV",
    "description": "",
    "body": "continueEmbeddedCodeV ${1:python}:\n   ${2:statement}${0}\nendEmbeddedCode"
  },
  {
    "trigger": "$onEmbeddedCode",
    "description": "",
    "body": "$onEmbeddedCode ${1:python}:\n   ${2:statement}${0}\n$offEmbeddedCode"
  },
  {
    "trigger": "$onEmbeddedCodeS",
    "description": "",
    "body": "$onEmbeddedCodeS ${1:python}:\n   ${2:statement}${0}\n$offEmbeddedCode"
  },
  {
    "trigger": "$onEmbeddedCodeV",
    "description": "",
    "body": "$onEmbeddedCodeV ${1:python}:\n   ${2:statement}${0}\n$offEmbeddedCode"
  },
  {
    "trigger": "$onEcho",
    "description": "",
    "body": "$onEcho > ${1:file}\n${2:output}${0}\n$offEcho"
  },
  {
    "trigger": "$onEchoS",
    "description": "",
    "body": "$onEchoS > ${1:file}\n${2:output}${0}\n$offEcho"
  },
  {
    "trigger": "$onEchoV",
    "description": "",
    "body": "$onEchoV > ${1:file}\n${2:output}${0}\n$offEcho"
  },
  {
    "trigger": "$onPut",
    "description": "",
    "body": "$onPut\n${1:text}${0}\n$offPut"
  },
  {
    "trigger": "$onPutS",
    "description": "",
    "body": "$onPutS\n${1:text}${0}\n$offPut"
  },
  {
    "trigger": "$onPutV",
    "description": "",
    "body": "$onPutV\n${1:text}${0}\n$offPut"
  },
  {
    "trigger": "$onExternalInput",
    "description": "",
    "body": "$onExternalInput\n${1:text}${0}\n$offExternalInput"
  },
  {
    "trigger": "$onExternalOutput",
    "description": "",
    "body": "$onExternalOutput\n${1:text}${0}\n$offExternalOutput"
  },
  {
    "trigger": "$ifThen",
    "description": "",
    "body": "$ifThen ${1:condition}\n   ${2:statement};${0}\n$endIf"
  },
  {
    "trigger": "$ifThenI",
    "description": "",
    "body": "$ifThenI ${1:condition}\n   ${2:statement};${0}\n$endIf"
  },
  {
    "trigger": "$ifThenE",
    "description": "",
    "body": "$ifThenE ${1:condition}\n   ${2:statement};${0}\n$endIf"
  },
  {
    "trigger": "$onFold",
    "description": "",
    "body": "$onFold\n${1:code}${0}\n$offFold"
  },
  {
    "trigger": "$onText",
    "description": "",
    "body": "$onText\n${1:comment}${0}\n$offText"
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

static const char *userJson = R"([
  {
    "usage": "
     === How to define your own snippets ===

    Below you see two examples for code snippets currently inactive. Remove '_' before 'trigger' will make them
    available in GAMS Studio.

    1. The 'trigger' defines the text that will make the snippet available in the code completer. If the last character
       is a digit it is ignored, this allows multiple definitions of the same keyword.

    2. The 'description' is shown right after the keyword in the code completer to distinct multi defined keywords.

    3. The 'body' is the content of the snippet. After insertion the completer focuses placeholder 1 and switches
       to the next with the TAB key. On ENTER the snippet control is left at the position of placeholder 0 if defined
       or on the last placeholder otherwise.
  "
  },
  {
    "_trigger": "example1",
    "description": "",
    "body": "example1 (${1:i} = ${2:1} to ${3:10},\n   ${4:statement};${0}\n);"
  },
  {
    "_trigger": "example2",
    "description": "",
    "body": "example2 (${1:x}, ${2:y})${0}"
  }
])";

inline void createUserSnippetFile(const QString &snippetFile)
{

    QFile f(snippetFile);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write(userJson);
        f.close();
        DEB() << "Default user snippets created at " << snippetFile;
    } else
        DEB() << "Could not write snippets to " << snippetFile;
}

inline const QByteArray getDefaultSnippets()
{
    const QMap<QByteArray, QByteArray> map = {{"[1]", "a"}, {"[2]", "b"}, {"[3]", "c"}};
    int max = 0;
    for (const QByteArray &val : map.values())
        max = qMax(max, val.length());

    QByteArray res = defaultJson;
    QByteArray spaces(max, ' ');
    res.replace("[0]", spaces);

    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        QByteArray uniSizeVal = it.value() + QByteArray(max - it.value().size(), ' ');
        res.replace(it.key()+'_', uniSizeVal);
        res.replace(it.key(), it.value());
    }
    return res;
}

inline int getSnippetsVersion(const QByteArray &snippets)
{
    int res = -1;
    QRegularExpression versionRegex(R"("version"\s*:\s*"([^"]+)");
    QRegularExpressionMatch match = versionRegex.match(QString::fromUtf8(snippets));
    if (match.hasMatch()) {
        bool ok = false;
        res = match.captured(1).toInt(&ok);
        return ok ? res : -1;
    }
    return -1;
}
