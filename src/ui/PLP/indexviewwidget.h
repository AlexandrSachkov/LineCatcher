#ifndef INDEXVIEWWIDGET_H
#define INDEXVIEWWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QPaintEvent>
#include <QMouseEvent>

#include "coreobjptr.h"
#include "pagedfileviewwidget.h"
#include "ullspinbox.h"
#include "IndexReaderI.h"
#include "FileReaderI.h"
#include <memory>

class IndexViewWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    IndexViewWidget(
        CoreObjPtr<PLP::IndexReaderI> indexReader,
        PagedFileViewWidget* fileViewer,
        ULLSpinBox* lineNavBox,
        QWidget* parent = nullptr
    );

protected:
    void resizeEvent(QResizeEvent *e) override;
public slots:
    void gotoLine(unsigned long long lineNum);
private slots:
    void textChangedImpl();
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect&, int);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;
    void readBlockIfRequired();
    void scrollBarMoved(int val);

private:
    void calcNumVisibleLines();
    void readNextBlock();
    void readPreviousBlock();
    void mouseReleaseEvent(QMouseEvent* event) override;

    static const unsigned int MAX_NUM_BLOCKS = 1000;
    static const unsigned int NUM_LINES_PER_READ = 250;

    CoreObjPtr<PLP::IndexReaderI> _indexReader;
    PagedFileViewWidget* _fileViewer;
    QWidget* _lineNumberArea;
    ULLSpinBox* _lineNavBox;
    std::vector<unsigned long long> _indices;
    std::vector<QString> _data;

    unsigned long long _startLineNum = 0;
    unsigned long long _endLineNum = 0;
    unsigned int _numVisibleLines = 0;
};

#endif // INDEXVIEWWIDGET_H
