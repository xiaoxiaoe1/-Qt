#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 主窗口负责三件事：
// 1. 建立/检查数据库；
// 2. 初始化主导航和页面切换；
// 3. 在启动时把学生数据同步到学生信息页。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void loadStudentData();

private:
    void initializeDatabase();
    void initializeNavigation();
    void initializeWindowText();

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
