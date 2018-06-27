#ifndef SYSTEMLOGEDIT_H
#define SYSTEMLOGEDIT_H

#include "abstractedit.h"

#include <QSyntaxHighlighter>
#include <QRegularExpression>

#include <QDebug>

namespace gams {
namespace studio {

class StudioSettings;

enum class LogMsgType { Error, Warning, Info };

class HighlightingRule
{
public:
    HighlightingRule() {}

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
        mPattern = QRegularExpression("Error:");
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
        mPattern = QRegularExpression("Info:");
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
        mPattern = QRegularExpression("Warning:");
        mFormat.setForeground(Qt::darkYellow);
        mFormat.setFontWeight(QFont::Bold);
    }
};


class SystemLogHighlighter
    : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SystemLogHighlighter(QObject *parent = nullptr)
        : QSyntaxHighlighter(parent)
    {
        mHighlightingRules.push_back(ErrorHighlightingRule());
        mHighlightingRules.push_back(InfoHighlightingRule());
        mHighlightingRules.push_back(WarningHighlightingRule());
        mHighlightingRules.push_back(LinkHighlightingRule());
    }

protected:
    void highlightBlock(const QString &text) override
    {
        for (const HighlightingRule &rule : mHighlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern().globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                qDebug() << match.captured();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format());
            }
        }
    }

private:
    QVector<HighlightingRule> mHighlightingRules;
};

class SystemLogEdit : public AbstractEdit
{
public:
    SystemLogEdit(StudioSettings *settings, QWidget *parent);
    void appendLog(const QString &msg, LogMsgType type = LogMsgType::Warning);

    EditorType type() override;

private:
    QString level(LogMsgType type);

private:
    SystemLogHighlighter *mHighlighter;
};

}
}

#endif // SYSTEMLOGEDIT_H
