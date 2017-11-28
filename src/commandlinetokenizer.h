#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QtWidgets>
#include "optcc.h"

namespace gams {
namespace studio {

struct OptionItem {
    OptionItem(QString k, QString v, unsigned int kpos, unsigned int vpos) :
          key(k), value(v), keyPosition(kpos),valuePosition(vpos) { }

    QString key;
    QString value;
    bool disabled = false;
    int keyPosition;
    int valuePosition;
};

struct OptionGroup {
    OptionGroup(QString n, int num, QString desc, int helpCtxt) :
         name(n), number(num), description(desc), helpContext(helpCtxt) { }

    QString name;
    int number;
    QString description;
    int helpContext;
};

struct OptionValue {
    OptionValue(QVariant val, QString desc, bool h, bool enumFlg) :
         value(val), description(desc), hidden(h), enumFlag(enumFlg) { }

    QVariant value;
    QString description;
    bool hidden;
    bool enumFlag;
};

struct OptionDefinition {
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

class CommandLineTokenizer
{
public:
    CommandLineTokenizer();

    QList<OptionItem> tokenize(const QString &commandLineStr);
    QList<QTextLayout::FormatRange> format(const QList<OptionItem> &items);

    void readDefinition(QString systemPath, QString optionFileName);
    void dumpOptionDefinition();

private:
    QTextCharFormat mInvalidKeyFormat;
    QTextCharFormat mInvalidValueFormat;

    QList<OptionDefinition> mOption;
    QMap<QString, QString> mSynonymMap;
    QMap<QString, QString> mDeprecatedMap;
    QMap<int, QString> mOptionTypeNameMap;
    QList<OptionGroup> mOptionGroupList;
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEPARSER_H
