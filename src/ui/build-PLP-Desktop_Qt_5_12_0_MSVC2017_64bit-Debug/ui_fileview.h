/********************************************************************************
** Form generated from reading UI file 'fileview.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILEVIEW_H
#define UI_FILEVIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FileView
{
public:
    QGridLayout *gridLayout;
    QPushButton *pushButton;
    QPushButton *pushButton_2;

    void setupUi(QWidget *FileView)
    {
        if (FileView->objectName().isEmpty())
            FileView->setObjectName(QString::fromUtf8("FileView"));
        FileView->resize(400, 300);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(FileView->sizePolicy().hasHeightForWidth());
        FileView->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(FileView);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        pushButton = new QPushButton(FileView);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        gridLayout->addWidget(pushButton, 0, 0, 1, 1);

        pushButton_2 = new QPushButton(FileView);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        gridLayout->addWidget(pushButton_2, 1, 0, 1, 1);


        retranslateUi(FileView);

        QMetaObject::connectSlotsByName(FileView);
    } // setupUi

    void retranslateUi(QWidget *FileView)
    {
        FileView->setWindowTitle(QApplication::translate("FileView", "Form", nullptr));
        pushButton->setText(QApplication::translate("FileView", "PushButton", nullptr));
        pushButton_2->setText(QApplication::translate("FileView", "PushButton", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FileView: public Ui_FileView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILEVIEW_H
