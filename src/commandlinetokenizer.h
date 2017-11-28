#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QtWidgets>

namespace gams {
namespace studio {

struct OptionItem {
    OptionItem(QString k, QString v, unsigned int kpos, unsigned int vpos)
         : key(k), value(v), keyPosition(kpos),valuePosition(vpos) { }

    QString key;
    QString value;
    int keyPosition;
    int valuePosition;
};

class CommandLineTokenizer
{
public:
    CommandLineTokenizer();

    QList<OptionItem> tokenize(const QString &commandLineStr);
    QList<QTextLayout::FormatRange> format(const QList<OptionItem> &items);

private:
    QTextCharFormat mInvalidKeyFormat;
    QTextCharFormat mInvalidValueFormat;

};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEPARSER_H
