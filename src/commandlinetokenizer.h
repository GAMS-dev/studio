#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QtWidgets>
#include "option.h"

namespace gams {
namespace studio {

enum OptionErrorType {
    InvalidKey,
    InvalidValue,
    NoKey,
    NoValue,
    Deprecated,
    unknown,
};

struct OptionError {
    OptionError() { }
    OptionError(QTextLayout::FormatRange fr, OptionErrorType t):
         formatRange(fr), type(t) { }

    QTextLayout::FormatRange formatRange;
    OptionErrorType type;
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

};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEPARSER_H
