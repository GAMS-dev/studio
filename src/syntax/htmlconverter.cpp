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
    QString backColor;
#ifndef __APPLE__
    backColor = QString("background-color: %1;").arg(background.name());
#endif
    res.append(QString("<html><body>\n<!--StartFragment--><div style=\"color: #d4d4d4;"
                       "font-family: Consolas, 'Courier New', monospace;font-weight: normal;"
                       "font-size: 14px;line-height: 19px;white-space: pre;%1\"><div>").arg(backColor).toUtf8());
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
        const QTextLayout::FormatRange *range = nullptr;
        int ri = -1;
        while (block.layout()->formats().size() > ri+1) {
            ++ri;
            range = &block.layout()->formats().at(ri);
            if (range->start + range->length >= i) break;
            if (ri+1 == block.layout()->formats().size()) {
                range = nullptr;
                ri = -1;
                break;
            }
        }
        if (block.text().isEmpty() && i < end) {
            res.append("<br>");
        } else {
            while (i < end) {
                if (range && range->start <= i) {
                    int to = qMin(range->start + range->length, end);
                    res.append(QString("<span style=\"color: %1;\">").arg(range->format.foreground().color().name()).toUtf8());
                    if (range->format.fontWeight() == QFont::Bold)
                        res.append("<strong>");
                    if (range->format.fontItalic())
                        res.append("<em>");

                    res.append(block.text().mid(i, to-i).toHtmlEscaped().toUtf8());

                    if (range->format.fontItalic())
                        res.append("</em>");
                    if (range->format.fontWeight() == QFont::Bold)
                        res.append("</strong>");
                    res.append("</span>");
                    i = to;
                    if (ri >= 0) {
                        if (ri+1 < block.layout()->formats().size()) {
                            range = &block.layout()->formats().at(++ri);
                        } else {
                            ri = -1;
                            range = nullptr;
                        }
                    }
                    if (to >= end) break;
                } else {
                    int to = (range ? range->start : end);
                    res.append(QString("<span style=\"color: %1;\">").arg(block.blockFormat().foreground().color().name()).toUtf8());
                    res.append(block.text().mid(i, to-i).toHtmlEscaped().toUtf8());
                    res.append("</span>");
                    i = to;
                    if (to >= end) break;
                }
            }
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
