#include "htmlconverter.h"
#include "logger.h"

#include <QTextDocument>
#include <QTextBlock>

namespace gams {
namespace studio {

HtmlConverter::HtmlConverter()
{}

QByteArray HtmlConverter::toHtml(QTextCursor cursor, QColor background)
{
    QByteArray res;
    if (!cursor.hasSelection()) return res;
    res.append(QString("<html>\n<body>\n<!--StartFragment--><div style=\"color: #d4d4d4;"
                       "background-color: %1;font-family: Consolas, 'Courier New', monospace;font-weight: normal;"
                       "font-size: 14px;line-height: 19px;white-space: pre;\"><div>").arg(background.name()).toUtf8());
    QTextDocument *doc = cursor.document();

    QTextCursor cur(doc);
    cur.setPosition(qMax(cursor.position(), cursor.anchor()));
    int lastEnd = cur.positionInBlock();
    QTextBlock lastBlock = cur.block();
    cur.setPosition(qMin(cursor.position(), cursor.anchor()));
    QTextBlock firstBlock = cur.block();
    QTextBlock block = firstBlock;
    int i = cur.positionInBlock();
    while (block.isValid()) {
        int end = (block == lastBlock) ? lastEnd : block.length();
        for (const QTextLayout::FormatRange &range : block.layout()->formats()) {
            if (range.start > i) continue;
            int to = qMin(range.start + range.length, end);
            if (to <= i) continue;

            res.append(QString("<span style=\"color: %1;\">").arg(range.format.foreground().color().name()).toUtf8());
            if (range.format.fontWeight() == QFont::Bold)
                res.append("<strong>");
            if (range.format.fontItalic())
                res.append("<em>");

            res.append(block.text().mid(i, to-i).toHtmlEscaped().toUtf8());

            if (range.format.fontItalic())
                res.append("</em>");
            if (range.format.fontWeight() == QFont::Bold)
                res.append("</strong>");
            res.append("</span>");
            i = to;
            if (to >= end) break;
        }
        if (block == lastBlock) {
            block = QTextBlock();
        } else {
            block = block.next();
            res.append("</div><div>");
        }
        i = 0;
    }
    res.append("</div></div><!--EndFragment-->\n</body>\n</html>");
    DEB() << res;
    return res;
}

QByteArray HtmlConverter::toHtml(QTextDocument *doc, QColor background)
{
    QTextCursor cur(doc);
    cur.select(QTextCursor::Document);
    return toHtml(cur, background);
}

} // namespace studio
} // namespace gams
