#ifndef STANDARDSEARCHVIEW_H
#define STANDARDSEARCHVIEW_H

#include <QWidget>
#include <QProgressDialog>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>

#include "ullspinbox.h"
#include "CoreI.h"
#include "coreobjptr.h"

class SearchView : public QWidget
{
    Q_OBJECT
public:
    explicit SearchView(PLP::CoreI* plpCore, bool multiline, QWidget *parent = nullptr);

    void createSourceContent(QLayout* mainLayout);
    void createDestinationContent(QLayout* mainLayout);
    void createSearchLimiterContent(QLayout* mainLayout);
    void createSearchOptionContent(QLayout* mainLayout);
    void createMultilineSearchOptionContent(QLayout* mainLayout);
    void setSourcePath(const QString& path);
signals:

private slots:
    void openFile();
    void openIndex();
    void openDestinationDir();
    void startSearch();
    void onSearchCancelled();
    void onSearchCompletion(bool success);
private:
    void showEvent(QShowEvent* event);

    void startRegularSearch(
            CoreObjPtr<PLP::FileReaderI> fileReader,
            CoreObjPtr<PLP::IndexReaderI> indexReader,
            CoreObjPtr<PLP::IndexWriterI> indexWriter,
            unsigned long long startLine,
            unsigned long long endLine,
            unsigned long long maxNumResults
            );

    void startMultilineSearch(
            CoreObjPtr<PLP::FileReaderI> fileReader,
            CoreObjPtr<PLP::IndexReaderI> indexReader,
            CoreObjPtr<PLP::IndexWriterI> indexWriter,
            unsigned long long startLine,
            unsigned long long endLine,
            unsigned long long maxNumResults
            );

    const int ROW_LABEL_WIDTH = 300;
    const int NUM_ROWS = 5;

    PLP::CoreI* _plpCore;
    bool _multiline = false;

    QLineEdit* _filePath;
    QLineEdit* _indexPath;
    QLineEdit* _destDir;
    QLineEdit* _destName;
    ULLSpinBox* _fromLineBox;
    ULLSpinBox* _toLineBox;
    ULLSpinBox* _numResultsBox;
    QLineEdit* _searchField;
    QCheckBox* _regex;

    std::vector<QCheckBox*> _lineEnabledCheckBoxes;
    std::vector<QSpinBox*> _lineOffsetBoxes;
    std::vector<QLineEdit*> _searchPatternBoxes;
    std::vector<QCheckBox*> _regexCheckBoxes;
};

#endif // STANDARDSEARCHVIEW_H
