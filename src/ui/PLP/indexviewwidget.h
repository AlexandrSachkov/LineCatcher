#ifndef INDEXVIEWWIDGET_H
#define INDEXVIEWWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QPaintEvent>

#include "FileReaderI.h"
#include <memory>

class IndexViewWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    IndexViewWidget(std::shared_ptr<PLP::FileReaderI> fileReader, QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void textChangedImpl();
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect&, int);
    void readBlockIfRequired();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;

private:
    void calcNumVisibleLines();
    void readNextBlock();
    void readPreviousBlock();

    static const unsigned int MAX_NUM_BLOCKS = 100;
    static const unsigned int NUM_LINES_PER_READ = 25;

    std::shared_ptr<PLP::FileReaderI> _fileReader;
    QWidget* _lineNumberArea;

    unsigned long long _startLineNum = 0;
    unsigned long long _endLineNum = 0;
    unsigned int _numVisibleLines = 0;
};

#endif // INDEXVIEWWIDGET_H
