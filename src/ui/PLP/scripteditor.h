#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "luasyntaxhighlighter.h"
#include <QPlainTextEdit>

class ScriptEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    ScriptEditor(QWidget* parent = nullptr);
    void setFontSize(int pointSize);
protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect&, int);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;

private:
    QWidget* _lineNumberArea;
    LuaSyntaxHighlighter* _highlighter;
};

#endif // SCRIPTEDITOR_H
