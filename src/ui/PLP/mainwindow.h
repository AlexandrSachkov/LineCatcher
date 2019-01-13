#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMenuBar>

/*namespace Ui {
class MainWindow;
}*/

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void closeTab(int index);
    void openFile();

private:
    QWidget* _centralWidget;
    QVBoxLayout* _mainLayout;
    QMenuBar* _menuBar;
    QAction* _openFile;
    QAction* _exit;

    //Ui::MainWindow *_ui;
    QTabWidget* _fileViewer;

};

#endif // MAINWINDOW_H
