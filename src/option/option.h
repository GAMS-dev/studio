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
#ifndef OPTION_H
#define OPTION_H

#include <QStringList>
#include <QMap>
#include <QVariant>
#include "optcc.h"

namespace gams {
namespace studio {
namespace option {

enum class OptionErrorType {
    No_Error,     // 0
    Invalid_Key,  // 1
    Incorrect_Value_Type,// 2
    Value_Out_Of_Range,  // 3
    Deprecated_Option,   // 4
    Override_Option,     // 5
    UserDefined_Error,   // 6
    Invalid_minVersion,  // 7
    Invalid_maxVersion,  // 8
    Missing_Value        // 9
};

enum class OptionDefinitionType {
    GamsOptionDefinition = 0,
    SolverOptionDefinition = 1,
    ConfigOptionDefinition = 2
};

inline const QString optionMimeType(OptionDefinitionType type) {
    switch (type) {
      case OptionDefinitionType::GamsOptionDefinition:  return "application/vnd.gams-pf.text";
      case OptionDefinitionType::SolverOptionDefinition:  return "application/vnd.solver-opt.text";
      case OptionDefinitionType::ConfigOptionDefinition:  return "application/vnd.gams-cfg.text";
   }
   return "";
}

struct OptionItem {
    OptionItem() { }
    OptionItem(int id, const QString &k, const QString &v) :
          optionId(id), key(k), value(v) { }
    OptionItem(const QString &k, const QString &v, int kpos, int vpos) :
          key(k), value(v), keyPosition(kpos),valuePosition(vpos) { }
    OptionItem(const QString &k, const QString &v, int kpos, int vpos, bool disabledFlag) :
          key(k), value(v), keyPosition(kpos),valuePosition(vpos), disabled(disabledFlag) { }

    int optionId = -1;
    QString key = "";
    QString value = "";
    int keyPosition = -1;
    int valuePosition = -1;
    bool disabled = false;
    bool recurrent = false;
    QList<int> recurrentIndices = QList<int>();
    OptionErrorType error = OptionErrorType::No_Error;
};

struct ParamConfigItem {
    ParamConfigItem() { }
    ParamConfigItem(int id, const QString &k, const QString &v) : optionId(id), key(k), value(v) {}
    ParamConfigItem(int id, const QString &k, const QString &v, const QString &min, const QString &max) :
        optionId(id), key(k), value(v), minVersion(min), maxVersion(max) {}

    int optionId = -1;
    QString key = "";
    QString value = "";
    QString minVersion = "";
    QString maxVersion =  "";
    bool disabled = false;
    bool recurrent = false;
    OptionErrorType error = OptionErrorType::No_Error;
};

struct SolverOptionItem {
    SolverOptionItem() { }
    SolverOptionItem(int id, const QString &k, const QString &v, const QString &t, bool disabledFlag) :
        optionId(id), key(k), value(v), text(t), disabled(disabledFlag) {}
    SolverOptionItem(int id, const QString &k, const QString &v, const QString &t, bool disabledFlag, OptionErrorType e) :
          optionId(id), key(k), value(v), text(t), disabled(disabledFlag), error(e) { }

    int optionId = -1;
    QString key = "";
    QString value = "";
    QString text = "";
    bool disabled = false;
    bool recurrent = false;
    OptionErrorType error = OptionErrorType::No_Error;
};

struct OptionGroup {
    OptionGroup() { }
    OptionGroup(const QString &n, int num, bool h, const QString &desc, int helpCtxt) :
         name(n), number(num), hidden(h), description(desc), helpContext(helpCtxt) { }

    QString name;
    int number;
    bool hidden;
    QString description;
    int helpContext;
};

struct OptionValue {
    OptionValue() { }
    OptionValue(const QVariant &val, const QString &desc, bool h, bool enumFlg) :
         value(val), description(desc), hidden(h), enumFlag(enumFlg) { }

    QVariant value;
    QString description;
    bool hidden;
    bool enumFlag;
};

struct OptionDefinition {
    OptionDefinition() { }
    OptionDefinition(int num, const QString &n, optDataType dt, optOptionType ot, optOptionSubType st, const QString &desc):
         number(num), name(n), dataType(dt), type(ot), subType(st), description(desc) { }

    int number;
    QString name;
    QString synonym;
    optDataType dataType;
    optOptionType type;
    optOptionSubType subType;
    QString description;
    bool deprecated = false;
    bool valid = false;
    QVariant defaultValue;
    QVariant lowerBound;
    QVariant upperBound;
    QList<OptionValue> valueList;
    int groupNumber;
    bool modified = false;
};

class Option
{
public:
    static const int GAMS_DEPRECATED_GROUP_NUMBER = 4;

    Option(const QString &optionFilePath, const QString &optionFileName);
    ~Option();

    void dumpAll();

    bool isValid(const QString &optionName) const;
    bool isSynonymDefined() const;
    bool isASynonym(const QString &optionName) const;
    bool isDeprecated(const QString &optionName) const;
    bool isDoubleDashedOption(const QString &option) const;
    bool isDoubleDashedOptionNameValid(const QString &optionName) const;
    bool isConformantVersion(const QString &version) const;
    OptionErrorType getValueErrorType(const QString &optionName, const QString &value) const;

    QString getNameFromSynonym(const QString &synonym) const;
    optOptionType getOptionType(const QString &optionName) const;
    optOptionSubType getOptionSubType(const QString &optionName) const;
    optDataType getDataType(const QString &optionName) const;
    QVariant getUpperBound(const QString &optionName) const;
    QVariant getLowerBound(const QString &optionName) const;
    QVariant getDefaultValue(const QString &optionName) const;
    QString getDescription(const QString &optionName) const;
    const QList<OptionValue> getValueList(const QString &optionName) const;

    const QString getEOLChars() const;
    bool isEOLCharDefined() const;

    QString getDefaultSeparator() const;
    bool isDefaultSeparatorDefined() const;

    QString getDefaultStringquote() const;
    bool isDefaultStringquoteDefined() const;

    QStringList getKeyList() const;
    QStringList getValidNonDeprecatedKeyList() const;
    QStringList getKeyAndSynonymList() const;
    QStringList getValuesList(const QString &optionName) const;
    const QStringList getSynonymList(const QString &optionName) const;
    QStringList getNonHiddenValuesList(const QString &optionName) const;

    int getOrdinalNumber(const QString &optionName) const;

    int getGroupNumber(const QString &optionName) const;
    bool isGroupHidden(int number) const;
    QString getGroupName(const QString &optionName) const;
    QString getGroupDescription(const QString &optionName) const;

    OptionDefinition getOptionDefinition(const QString &optionName) const;
    QList<OptionGroup> getOptionGroupList() const;
    QString getOptionTypeName(int type) const;
    QString getOptionKey(const QString &option) const;

    bool available() const;

    QMap<QString, OptionDefinition> getOption() const;

    bool isModified(const QString &optionName) const;
    void setModified(const QString &optionName, bool modified);
    void resetModficationFlag();

    QString getOptionDefinitionFile() const;
    QString getOptionDefinitionPath() const;

    static int errorCallback(int count, const char *message);

private:
    QString mOptionDefinitionPath;
    QString mOptionDefinitionFile;

    QString mEOLChars;
    QString mSeparator;
    QString mStringquote;

    QMap<QString, OptionDefinition> mOption;
    QStringList mDeprecatedSynonym;
    QMultiMap<QString, QString> mSynonymMap;
    QMap<int, QString> mOptionTypeNameMap;
    QMap<int, OptionGroup> mOptionGroup;

    static QRegularExpression mRexDoubleDash;
    static QRegularExpression mRexDoubleDashName;
    static QRegularExpression mRexVersion;
    static QRegularExpression mRexOptionKey;

    bool mAvailable;
    bool readDefinitionFile(const QString &optionFilePath, const QString &optionFileName);
};

const double OPTION_VALUE_MAXDOUBLE = 1e+299;
const double OPTION_VALUE_MINDOUBLE = -1e+299;
const int OPTION_VALUE_MAXINT = INT_MAX;
const int OPTION_VALUE_MININT = INT_MIN;
const int OPTION_VALUE_DECIMALS = 20;
const QString GamsOptDefFile = "optgams.def";

} // namespace option
} // namespace studio
} // namespace gams

Q_DECLARE_METATYPE(gams::studio::option::OptionItem)

#endif // OPTION_H
