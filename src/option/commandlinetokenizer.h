#ifndef COMMANDLINETOKENIZER_H
#define COMMANDLINETOKENIZER_H

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

    CommandLineTokenizer(Option* gamsOption);
    ~CommandLineTokenizer();

    QList<OptionItem> tokenize(const QString &commandLineStr);
    QList<OptionItem> tokenize(const QString &commandLineStr, const QList<QString> &disabledOption);
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

    Option *getGamsOption() const;

public slots:
    void formatTextLineEdit(QLineEdit* lineEdit, const QString &commandLineStr);
    void formatItemLineEdit(QLineEdit* lineEdit, const QList<OptionItem> &optionItems);

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

    void formatLineEdit(QLineEdit* lineEdit, const QList<OptionError> &errorList);
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINETOKENIZER_H
