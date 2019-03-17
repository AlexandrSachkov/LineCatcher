#ifndef LUASYNTAXHIGHLIGHTER_H
#define LUASYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class LuaSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    LuaSyntaxHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif // LUASYNTAXHIGHLIGHTER_H
