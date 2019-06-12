#ifndef PAGEDFILEVIEW_H
#define PAGEDFILEVIEW_H

#include "ullspinbox.h"
#include "coreobjptr.h"

#include <QPlainTextEdit>
#include <QScrollBar>
#include <QPaintEvent>
#include <QSpinBox>

#include "FileReaderI.h"
#include "IndexReaderI.h"
#include <memory>
#include <vector>

class PagedFileViewWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    PagedFileViewWidget(CoreObjPtr<PLP::FileReaderI> fileReader, ULLSpinBox* lineNavBox, QWidget *parent = nullptr);
    bool getLineFromIndex(
        CoreObjPtr<PLP::IndexReaderI>& indexReader,
        QString& data
    );

    void setFontSize(int pointSize);

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

    CoreObjPtr<PLP::FileReaderI> _fileReader;
    unsigned int _numVisibleLines;

    QWidget* _lineNumberArea;
    ULLSpinBox* _lineNavBox;
};

#endif // PAGEDFILEVIEW_H
