#include "moneywidget.h"
#include "ui_moneywidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QChartView>
#include <QChart>
#include <QTableWidget>
// 【关键1】必须同时包含这两个头文件
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
// ✅【必须加这行！99%是它漏了】
using namespace QtCharts;

MoneyWidget::MoneyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MoneyWidget)
{
    ui->setupUi(this);
    setupUI();
}

MoneyWidget::~MoneyWidget()
{
    delete ui;
}

void MoneyWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* topLayout = new QHBoxLayout();
    QHBoxLayout* middleLayyout = new QHBoxLayout();

    // ✅ 正常创建，不会报错
    chartView = new QChartView();

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(middleLayyout, 60);
    mainLayout->addWidget(chartView, 40);

    //===================顶部筛选条件与按钮布局=====
    topLayout->addWidget(new QLabel("学生姓名:", this));
    studentComboBox = new QComboBox(this);
    topLayout->addWidget(studentComboBox);

    topLayout->addWidget(new QLabel("起始日期"));
    startDateEdit = new QDateEdit(QDate::currentDate().addMonths(-1));
    startDateEdit->setCalendarPopup(true); //弹出日历
    topLayout->addWidget(startDateEdit);

    topLayout->addWidget(new QLabel("结束日期"));
    endDateEdit = new QDateEdit(QDate::currentDate());
    endDateEdit->setCalendarPopup(true);
    topLayout->addWidget(endDateEdit);

    addButton = new QPushButton("添加");
    deleteButton = new QPushButton("删除");
    editButton = new QPushButton("修改");
    topLayout->addWidget(addButton);
    topLayout->addWidget(deleteButton);
    topLayout->addWidget(editButton);
    topLayout->addStretch();

    //====================主页面布局==========
    tablewidget = new QTableWidget();
    tablewidget->setFixedWidth(750);
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setAlternatingRowColors(true);
    QStringList header = QStringList() << "ID" << "学生姓名" << "缴费日期" << "金额" << "支付类型" << "备注";
    tablewidget->setColumnCount(header.count());
    tablewidget->setHorizontalHeaderLabels(header);
    tablewidget->setColumnHidden(0, true);
    middleLayyout->addWidget(tablewidget);
    pieChartView = new QChartView();
    middleLayyout->addWidget(pieChartView);
    chartView->setRenderHint(QPainter::Antialiasing); //抗锯齿
    chartView->setMinimumHeight(200); //最小高度保障

    //连接
    //connect(addButton, &QPushButton::clicked, this, )

}
