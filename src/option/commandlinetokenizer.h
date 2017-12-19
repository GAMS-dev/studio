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

class CommandLineTokenizer : public QObject
{
    Q_OBJECT

public:

    CommandLineTokenizer();
    ~CommandLineTokenizer();

    QList<OptionItem> tokenize(const QString &commandLineStr);
    QList<OptionError> format(const QList<OptionItem> &items);
    QString normalize(const QString &commandLineStr);
    QString normalize(const QList<OptionItem> &items);

    QTextCharFormat invalidKeyFormat() const;
    QTextCharFormat invalidValueFormat() const;
    QTextCharFormat deprecateOptionFormat() const;

    void setInvalidKeyFormat(const QTextCharFormat &invalidKeyFormat);
    void setInvalidValueFormat(const QTextCharFormat &invalidValueFormat);
    void setDeprecateOptionFormat(const QTextCharFormat &deprecateOptionFormat);
    void setDeactivatedOptionFormat(const QTextCharFormat &deactivatedOptionFormat);

public slots:
    void formatLineEditTextFormat(QLineEdit* lineEdit, const QString &commandLineStr);

private:
    QTextCharFormat mInvalidKeyFormat;
    QTextCharFormat mInvalidValueFormat;
    QTextCharFormat mDeprecateOptionFormat;
    QTextCharFormat mDeactivatedOptionFormat;

    Option* gamsOption;

    void offsetWhiteSpaces(QStringRef str, int &offset, const int length);
    void offsetKey(QStringRef str,  QString &key, int &keyPosition, int &offset, const int length);
    void offsetAssignment(QStringRef str, int &offset, const int length);
    void offsetValue(QStringRef str, QString &value, int &valuePosition, int &offset, const int length);

    void setLineEditTextFormat(QLineEdit* lineEdit, const QString &commandLineStr);
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEPARSER_H
