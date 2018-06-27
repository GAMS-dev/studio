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

class SystemLogHighlighter
    : public QSyntaxHighlighter
{
    Q_OBJECT
private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

public:
    SystemLogHighlighter(QObject *parent = nullptr)
        : QSyntaxHighlighter(parent)
    {
//        QTextCharFormat errorFormat;
//        errorFormat.setForeground(Qt::red);
//        errorFormat.setFontWeight(QFont::Bold);
//        HighlightingRule errorRule {
//            QRegularExpression("Error:"),
//            errorFormat
//        };
        //mHighlightingRules.push_back(errorRule);

//        QTextCharFormat infoFormat;
//        infoFormat.setForeground(Qt::darkBlue);
//        infoFormat.setFontWeight(QFont::Bold);
//        HighlightingRule infoRule {
//            QRegularExpression("Info:"),
//            infoFormat
//        };
        //mHighlightingRules.push_back(infoRule);

//        QTextCharFormat warnFormat;
//        warnFormat.setForeground(Qt::darkYellow);
//        warnFormat.setFontWeight(QFont::Bold);
//        HighlightingRule warnRule {
//            QRegularExpression("Warning:"),
//            warnFormat
//        };
        //mHighlightingRules.push_back(warnRule);

        QTextCharFormat linkFormat;
        linkFormat.setForeground(Qt::blue);
        //linkFormat.setUnderlineColor(Qt::blue);
        linkFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        linkFormat.setAnchor(true);
        //linkFormat.setAnchorName("???")
        //linkFormat.setAnchorHref()
        HighlightingRule linkRule {
            QRegularExpression("http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+"),
            linkFormat
        };
        mHighlightingRules.push_back(linkRule);
    }

protected:
    void highlightBlock(const QString &text) override
    {
        QTextCursor tc(document());
        for (HighlightingRule &rule : mHighlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                qDebug() << match.captured();
                rule.format.setAnchorHref(match.captured());
                rule.format.setAnchorName(match.captured());
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
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
    SystemLogHighlighter *mHighlighter;
};

}
}

#endif // SYSTEMLOGEDIT_H
