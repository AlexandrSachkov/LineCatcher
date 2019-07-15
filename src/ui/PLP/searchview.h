/*
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
