#include "pagedfileviewwidget.h"
#include <QDebug>
#include <QPainter>
#include <QTextBlock>
#include <QtMath>
#include <QScrollBar>

PagedFileViewWidget::PagedFileViewWidget(std::unique_ptr<PLP::FileReaderI> fileReader, QWidget *parent) : QPlainTextEdit (parent)
{
    _fileReader = std::move(fileReader);
    _lineNumberArea = new LineNumberArea(this);
    connect(this, SIGNAL(textChanged(void)), this, SLOT(textChangedImpl(void)));
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));

    this->setLineWrapMode(LineWrapMode::NoWrap);
    this->setWordWrapMode(QTextOption::WrapMode::NoWrap);
    this->setMouseTracking(true);
    this->setMaximumBlockCount(MAX_NUM_BLOCKS);

    SignalingScrollBar* scrollBar = new SignalingScrollBar();
    connect(scrollBar, SIGNAL(mouseReleased(void)), this, SLOT(readBlockIfRequired(void)));
    connect(scrollBar, SIGNAL(wheelMoved(void)), this, SLOT(readBlockIfRequired(void)));
    this->setVerticalScrollBar(scrollBar);

    QFont f("Courier New", 16);
    f.setStyleHint(QFont::Monospace);
    this->setFont(f);

    calcNumVisibleLines();

    readNextBlock();
    this->moveCursor(QTextCursor::Start);
    this->ensureCursorVisible();

    updateLineNumberAreaWidth(0);
}

void PagedFileViewWidget::resizeEvent(QResizeEvent *e){
    QPlainTextEdit::resizeEvent(e);

    calcNumVisibleLines();
    qDebug() << "Resized" << "\n";

    QRect cr = contentsRect();
    _lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void PagedFileViewWidget::textChangedImpl() {
    qDebug() << "Text changed" << "\n";
    calcNumVisibleLines();
}

void PagedFileViewWidget::mouseMoveEvent(QMouseEvent *e) {
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow).lighter(160);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = this->cursorForPosition(e->pos());
    selection.cursor.clearSelection();

    this->setExtraSelections({selection});
}

void PagedFileViewWidget::calcNumVisibleLines() {
    _numVisibleLines = this->height() / this->fontMetrics().height();
    qDebug() << "Num lines: " <<_numVisibleLines <<"\n";
}

int PagedFileViewWidget::lineNumberAreaWidth()
{
    int digits = 1;
    unsigned long long max = _endLineNum > 1 ? _endLineNum : 1;
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}



void PagedFileViewWidget::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}



void PagedFileViewWidget::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy){
        _lineNumberArea->scroll(0, dy);
    } else {
        _lineNumberArea->update(0, rect.y(), _lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void PagedFileViewWidget::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(_startLineNum + blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, _lineNumberArea->width(), fontMetrics().height(), Qt::AlignLeft, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void PagedFileViewWidget::readNextBlock() {
    const int currScrollbarValue = this->verticalScrollBar()->value();
    const unsigned long long currStartLineNum = _startLineNum;

    for(int i = 0; i < NUM_LINES_PER_READ; i++){
        char* lineStart = nullptr;
        unsigned int length;
        if(!_fileReader->nextLine(lineStart, length)){
            break;
        }
        this->moveCursor(QTextCursor::End);
        this->insertPlainText(QString::fromUtf8(lineStart, length));

        _endLineNum++;
        _startLineNum = _endLineNum >= MAX_NUM_BLOCKS ? _endLineNum - MAX_NUM_BLOCKS : 0;
    }

    int newScrollbarValue = currScrollbarValue - (_startLineNum - currStartLineNum);
    this->verticalScrollBar()->setValue(newScrollbarValue);
}

void PagedFileViewWidget::readBlockIfRequired() {
    const int currScrollbarValue = this->verticalScrollBar()->value();
    const unsigned long long currStartLineNum = _startLineNum;

    if(_startLineNum + currScrollbarValue + _numVisibleLines > _endLineNum - (NUM_LINES_PER_READ / 2)){
        readNextBlock();
    }
}
