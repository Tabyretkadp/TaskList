#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_addTask_clicked();
    void on_showOn_clicked();

    void on_gradeTask_clicked();

    void on_h1_clicked();

    void on_h2_clicked();

    void on_h3_clicked();

    void on_pp_clicked();

    void on_showOff_clicked();

private:
    Ui::MainWindow *ui;

    int originalWidgetWidth = 0;
    int originalScrollWidth = 0;

    void loadTasks();
};
#endif // MAINWINDOW_H
