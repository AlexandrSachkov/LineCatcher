#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H
#include <QPlainTextEdit>

class ScriptEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    ScriptEditor(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect&, int);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;

private:
    QWidget* _lineNumberArea;
};

#endif // SCRIPTEDITOR_H
