#include "filenavcontrols.h"
#include <QtWidgets/QHBoxLayout>
#include <QLabel>

FileNavControls::FileNavControls(QWidget* parent) : QWidget(parent)
{
    QHBoxLayout* mainLayout = new QHBoxLayout();
    this->setLayout(mainLayout);

    QLabel* line = new QLabel("Line ", this);
    mainLayout->addWidget(line);


}
