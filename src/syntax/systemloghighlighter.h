#ifndef SYSTEMLOGHIGHLIGHTER_H
#define SYSTEMLOGHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>

namespace gams {
namespace studio {

class HighlightingData
{
private:
    HighlightingData() {}

public:
    static const QString ErrorKeyword;
    static const QString InfoKeyword;
    static const QString WarningKeyword;
};

class HighlightingRule
{
public:
    QRegularExpression pattern() const
    {
        return mPattern;
    }

    QTextCharFormat format() const
    {
        return mFormat;
    }

protected:
    QRegularExpression mPattern;
    QTextCharFormat mFormat;
    const QString timestampRegex = " \\[\\d\\d:\\d\\d:\\d\\d\\]:";
};

class LinkHighlightingRule
    : public HighlightingRule
{
public:
    LinkHighlightingRule()
    {
        mPattern = QRegularExpression("http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+");
        mFormat.setForeground(Qt::blue);
        mFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        mFormat.setAnchor(true);
    }
};

class ErrorHighlightingRule
    : public HighlightingRule
{
public:
    ErrorHighlightingRule()
    {
        mPattern = QRegularExpression(HighlightingData::ErrorKeyword + timestampRegex);
        mFormat.setForeground(Qt::red);
        mFormat.setFontWeight(QFont::Bold);
    }
};

class InfoHighlightingRule
    : public HighlightingRule
{
public:
    InfoHighlightingRule()
    {
        mPattern = QRegularExpression(HighlightingData::InfoKeyword + timestampRegex);
        mFormat.setForeground(Qt::darkBlue);
        mFormat.setFontWeight(QFont::Bold);
    }
};

class WarningHighlightingRule
    : public HighlightingRule
{
public:
    WarningHighlightingRule()
    {
        mPattern = QRegularExpression(HighlightingData::WarningKeyword + timestampRegex);
        mFormat.setForeground(Qt::darkYellow);
        mFormat.setFontWeight(QFont::Bold);
    }
};


class SystemLogHighlighter
    : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SystemLogHighlighter(QObject *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    QVector<HighlightingRule> mHighlightingRules;
};

}
}

#endif // SYSTEMLOGHIGHLIGHTER_H
