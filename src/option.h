#ifndef OPTION_H
#define OPTION_H

#include <QtWidgets>
#include "optcc.h"

namespace gams {
namespace studio {

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

    Option(QString systemPath, QString optionFileName);
    ~Option();

    void dumpAll();

    bool isValid(QString optionName);
    bool isDeprecated(QString optionName);
    QString getSynonym(QString optionName) const;
    QVariant getDefaultValue(QString optionName) const;
    QString getDescription(QString optionName) const;

    OptionDefinition getOptionDefinition(QString optionName) const;
    QList<OptionGroup> getOptionGroupList() const;
    QMap<int, QString> getOptionTypeNameMap() const;

private:
    QMap<QString, OptionDefinition> mOption;
    QMap<QString, QString> mSynonymMap;
    QMap<QString, QString> mDeprecatedMap;
    QMap<int, QString> mOptionTypeNameMap;
    QList<OptionGroup> mOptionGroupList;

    void readDefinition(QString systemPath, QString optionFileName);
};

} // namespace studio
} // namespace gams

#endif // OPTION_H
