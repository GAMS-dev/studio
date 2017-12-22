#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QtWidgets>
#include "option.h"

namespace gams {
namespace studio {

struct OptionError {
    OptionError() { }
    OptionError(QTextLayout::FormatRange fr, QString m):
         formatRange(fr), message(m) { }

    QTextLayout::FormatRange formatRange;
    QString message;
};

class CommandLineTokenizer
{
public:

    CommandLineTokenizer();
    ~CommandLineTokenizer();

    QList<OptionItem> tokenize(const QString &commandLineStr);
    QList<OptionError> format(const QList<OptionItem> &items);

    QTextCharFormat invalidKeyFormat() const;
    QTextCharFormat invalidValueFormat() const;
    QTextCharFormat deprecateOptionFormat() const;

private:
    QTextCharFormat mInvalidKeyFormat;
    QTextCharFormat mInvalidValueFormat;
    QTextCharFormat mDeprecateOptionFormat;

    Option* gamsOption;

    void offsetWhiteSpaces(QStringRef str, int &offset, const int length);
    void offsetKey(QStringRef str,  QString &key, int &keyPosition, int &offset, const int length);
    void offsetAssignment(QStringRef str, int &offset, const int length);
    void offsetValue(QStringRef str, QString &value, int &valuePosition, int &offset, const int length);
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEPARSER_H
