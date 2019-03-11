#ifndef PAGEDFILEVIEW_H
#define PAGEDFILEVIEW_H

#include "ullspinbox.h"

#include <QPlainTextEdit>
#include <QScrollBar>
#include <QPaintEvent>
#include <QSpinBox>

#include "FileReaderI.h"
#include "ResultSetReaderI.h"
#include <memory>
#include <vector>

class PagedFileViewWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    PagedFileViewWidget(std::unique_ptr<PLP::FileReaderI> fileReader, ULLSpinBox* lineNavBox, QWidget *parent = nullptr);
    bool getLineFromIndex(
        std::unique_ptr<PLP::ResultSetReaderI>& indexReader,
        QString& data
    );

signals:

public slots:
    void gotoLine(unsigned long long lineNum);

private slots:
    void textChangedImpl();
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect&, int);
    void readBlockIfRequired();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;
    void scrollBarMoved(int val);

protected:
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    void calcNumVisibleLines();
    void readNextBlock();
    void readPreviousBlock();

    static const unsigned int MAX_NUM_BLOCKS = 1000;
    static const unsigned int NUM_LINES_PER_READ = 250;

    unsigned long long _startLineNum = 0;
    unsigned long long _endLineNum = 0;

    std::unique_ptr<PLP::FileReaderI> _fileReader;
    unsigned int _numVisibleLines;

    QWidget* _lineNumberArea;
    ULLSpinBox* _lineNavBox;
};

#endif // PAGEDFILEVIEW_H