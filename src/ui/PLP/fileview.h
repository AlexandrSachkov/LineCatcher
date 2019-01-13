#ifndef FILEVIEW_H
#define FILEVIEW_H

#include "filenavcontrols.h"

#include <QWidget>
#include <QTextEdit>
#include <QTabWidget>
#include <QLabel>


class FileView : public QWidget
{
    Q_OBJECT
public:
    explicit FileView(const QString& filePath, QWidget *parent = nullptr);
    ~FileView();
signals:

public slots:

private:
    QTextEdit* _dataView;
    QTabWidget* _searchResults;
    QLabel* _fileInfo;
    FileNavControls* _fileNavControls;
};

#endif // FILEVIEW_H
