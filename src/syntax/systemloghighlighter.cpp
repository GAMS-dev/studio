#include "systemloghighlighter.h"

namespace gams {
namespace studio {

const QString HighlightingData::ErrorKeyword = "Error:";
const QString HighlightingData::InfoKeyword = "Info:";
const QString HighlightingData::WarningKeyword = "Warning:";

SystemLogHighlighter::SystemLogHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    mHighlightingRules.push_back(ErrorHighlightingRule());
    mHighlightingRules.push_back(InfoHighlightingRule());
    mHighlightingRules.push_back(WarningHighlightingRule());
    mHighlightingRules.push_back(LinkHighlightingRule());
}

void SystemLogHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : mHighlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern().globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format());
        }
    }
}

}
}
