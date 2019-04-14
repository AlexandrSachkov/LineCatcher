#ifndef STANDARDSEARCHVIEW_H
#define STANDARDSEARCHVIEW_H

#include <QWidget>
#include <QProgressDialog>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>

#include "ullspinbox.h"
#include "CoreI.h"

class SearchView : public QWidget
{
    Q_OBJECT
public:
    explicit SearchView(PLP::CoreI* plpCore, QWidget *parent = nullptr);

    void createSourceContent(QLayout* mainLayout);
    void createDestinationContent(QLayout* mainLayout);
    void createSearchLimiterContent(QLayout* mainLayout);
    void createSearchOptionContent(QLayout* mainLayout);
    void createMultilineSearchOptionContent(QLayout* mainLayout);
signals:
    void progressUpdate(int percent, unsigned long long numResults);
    void searchError();
private slots:
    void openFile();
    void openIndex();
    void startSearch();
    void onProgressUpdate(int percent, unsigned long long numResults);
    void onSearchError();
private:
    const int ROW_LABEL_WIDTH = 300;
    PLP::CoreI* _plpCore;
    QProgressDialog* _progressDialog = nullptr;

    QLineEdit* _filePath;
    QLineEdit* _indexPath;
    QLineEdit* _destName;
    ULLSpinBox* _fromLineBox;
    ULLSpinBox* _toLineBox;
    ULLSpinBox* _numResultsBox;
    QLineEdit* _searchField;
    QRadioButton* _plainText;
    QRadioButton* _regex;
    QCheckBox* _ignoreCase;
};

#endif // STANDARDSEARCHVIEW_H
