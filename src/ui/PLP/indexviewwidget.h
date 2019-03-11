#ifndef INDEXVIEWWIDGET_H
#define INDEXVIEWWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QPaintEvent>
#include <QMouseEvent>

#include "pagedfileviewwidget.h"
#include "ResultSetReaderI.h"
#include "FileReaderI.h"
#include <memory>

class IndexViewWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    IndexViewWidget(std::unique_ptr<PLP::ResultSetReaderI> indexReader, PagedFileViewWidget* fileViewer, QWidget* parent = nullptr);

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
    void mousePressEvent(QMouseEvent* event) override;

    static const unsigned int MAX_NUM_BLOCKS = 1000;
    static const unsigned int NUM_LINES_PER_READ = 250;

    std::unique_ptr<PLP::ResultSetReaderI> _indexReader;
    PagedFileViewWidget* _fileViewer;
    QWidget* _lineNumberArea;
    std::vector<unsigned long long> _indices;
    std::vector<QString> _data;

    unsigned long long _startLineNum = 0;
    unsigned long long _endLineNum = 0;
    unsigned int _numVisibleLines = 0;
};

#endif // INDEXVIEWWIDGET_H
