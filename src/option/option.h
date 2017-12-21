#ifndef OPTION_H
#define OPTION_H

#include <QtWidgets>
#include "optcc.h"

namespace gams {
namespace studio {

enum OptionErrorType {
    No_Error,
    Invalid_Key,
    Incorrect_Value_Type,
    Value_Out_Of_Range,
    Deprecated_Option,
    Unknown_Error
};

struct OptionItem {
    OptionItem() { }
    OptionItem(QString k, QString v, unsigned int kpos, unsigned int vpos) :
          key(k), value(v), keyPosition(kpos),valuePosition(vpos) { }

    QString key;
    QString value;
    bool disabled = false;
    int keyPosition;
    int valuePosition;
};

struct OptionGroup {
    OptionGroup() { }
    OptionGroup(QString n, int num, QString desc, int helpCtxt) :
         name(n), number(num), description(desc), helpContext(helpCtxt) { }

    QString name;
    int number;
    QString description;
    int helpContext;
};

struct OptionValue {
    OptionValue() { }
    OptionValue(QVariant val, QString desc, bool h, bool enumFlg) :
         value(val), description(desc), hidden(h), enumFlag(enumFlg) { }

    QVariant value;
    QString description;
    bool hidden;
    bool enumFlag;
};

struct OptionDefinition {
    OptionDefinition() { }
    OptionDefinition(QString n, optOptionType ot, optDataType dt, QString desc):
         name(n), type(ot), dataType(dt), description(desc) { }

    QString name;
    QString synonym;
    optOptionType type;
    optDataType dataType;
    QString description;
    bool deprecated;
    bool valid;
    QVariant defaultValue;
    QVariant lowerBound;
    QVariant upperBound;
    QList<OptionValue> valueList;
    int groupNumber;
};


class Option
{
public:
    static const int GAMS_DEPRECATED_GROUP_NUMBER = 4;

    Option(const QString &systemPath, const QString &optionFileName);
    ~Option();

    void dumpAll();

    bool isValid(const QString &optionName);
    bool isThereASynonym(const QString &optionName);
    bool isDeprecated(const QString &optionName);
    bool isDoubleDashedOption(const QString &optionName);
    OptionErrorType getValueErrorType(const QString &optionName, const QString &value);

    QString getSynonym(const QString &optionName) const;
    optOptionType getOptionType(const QString &optionName) const;
    optDataType getDataType(const QString &optionName) const;
    QVariant getUpperBound(const QString &optionName) const;
    QVariant getLowerBound(const QString &optionName) const;
    QVariant getDefaultValue(const QString &optionName) const;
    QString getDescription(const QString &optionName) const;
    QList<OptionValue> getValueList(const QString &optionName) const;

    QStringList getKeyList() const;
    QStringList getKeyAndSynonymList() const;
    QStringList getValuesList(const QString &optionName) const;

    OptionDefinition getOptionDefinition(const QString &optionName) const;
    QList<OptionGroup> getOptionGroupList() const;
    QString getOptionTypeName(int type) const;

    bool available() const;

private:
    QMap<QString, OptionDefinition> mOption;
    QMap<QString, QString> mSynonymMap;
//    QMap<QString, QString> mDeprecatedMap;
    QMap<int, QString> mOptionTypeNameMap;
    QList<OptionGroup> mOptionGroupList;

    bool mAvailable;
    bool readDefinition(const QString &systemPath, const QString &optionFileName);
};

} // namespace studio
} // namespace gams

#endif // OPTION_H
