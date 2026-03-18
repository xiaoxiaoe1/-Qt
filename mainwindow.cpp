#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QButtonGroup *bgp = new QButtonGroup(this);
    bgp->addButton(ui->tbnstdInfo,0);
    bgp->addButton(ui->tbnClass,1);
    bgp->addButton(ui->tbnMoney,2);
    bgp->addButton(ui->tbnHonor,3);
    bgp->addButton(ui->tbnSystem,4);

    // ✅【这行是关键！】点击按钮发送ID，切换页面
    connect(bgp, SIGNAL(buttonClicked(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
    //默认第一页
    bgp->button(0)->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);

}

MainWindow::~MainWindow()
{
    delete ui;
}

