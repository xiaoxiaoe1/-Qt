#include "moneywidget.h"
#include "ui_moneywidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QHeaderView>  // 加上这一行！
#include <QDebug>
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFormLayout>
#include <QSqlError>
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
    // 先填充学生下拉列表
    populateStudentComboBox();
    // 在setupUI最后添加调试：打印表格的可见性和尺寸
    qDebug() << "表格可见性：" << tablewidget->isVisible();
    qDebug() << "表格尺寸：" << tablewidget->size();
    qDebug() << "表格行数：" << tablewidget->rowCount(); // 看查询后是否有行数
    // 再加载财务数据
    locadMoney();

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
    tablewidget = new QTableWidget(this);// 显式指定父控件，避免布局丢失
    tablewidget->setFixedWidth(750);
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setAlternatingRowColors(true);
    QStringList header = QStringList() << "ID" << "学生姓名" << "缴费日期" << "金额" << "支付类型" << "备注";
    tablewidget->setColumnCount(header.count());
    tablewidget->setHorizontalHeaderLabels(header);
    tablewidget->setColumnHidden(0, true);
    // 强制设置表格可见（关键！）
    tablewidget->setVisible(true);
    // 给表格设置最小高度，避免被压缩
    tablewidget->setMinimumHeight(400);
    // 修复布局添加顺序 + 确保布局生效
    middleLayyout->addWidget(tablewidget, 1);// 加拉伸因子，让表格占满空间
    pieChartView = new QChartView(this);
    middleLayyout->addWidget(pieChartView, 1);

    chartView->setRenderHint(QPainter::Antialiasing); //抗锯齿
    chartView->setMinimumHeight(200); //最小高度保障
    // 【关键】将布局设置给当前widget（确保布局生效）
    this->setLayout(mainLayout);

    //连接
    connect(addButton, &QPushButton::clicked, this, &MoneyWidget::addRecord);
    //修改按钮
    connect(editButton, &QPushButton::clicked, this, &MoneyWidget::editRecord);
    //删除按钮
    connect(deleteButton, &QPushButton::clicked, this, &MoneyWidget::deleteRecord);
    // 连接学生下拉框的信号，当选择变化时更新表格数据
    connect(studentComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MoneyWidget::locadMoney);
    // 连接日期选择器的信号，当日期变化时更新表格数据
    connect(startDateEdit, &QDateEdit::dateChanged, this, &MoneyWidget::locadMoney);
    connect(endDateEdit, &QDateEdit::dateChanged, this, &MoneyWidget::locadMoney);

}

void MoneyWidget::locadMoney()
{
    tablewidget->setRowCount(0);

    QString studentId = studentComboBox->currentData().toString();
    QDate startDate = startDateEdit->date();
    QDate endDate = endDateEdit->date();

    // 打印调试信息
    qDebug() << "查询学生ID：" << studentId;
    qDebug() << "查询开始日期：" << startDate.toString("yyyy-MM-dd");
    qDebug() << "查询结束日期：" << endDate.toString("yyyy-MM-dd");

    QString queryStr = QString(
                "SELECT fr.id, s.name, fr.payment_date, fr.amount, fr.payment_type, fr.notes "
                "FROM financialRecords fr "
                "JOIN student s ON fr.student_id = s.student_id "
                "WHERE fr.payment_date BETWEEN ? AND ? %1 "
                ).arg((studentId != "-1") ? "AND fr.student_id = ?" : "");

    QSqlQuery query;
    query.prepare(queryStr);
    // 绑定日期参数
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));
    // 绑定学生ID（如果有筛选）
    if (studentId != "-1") {
        query.addBindValue(studentId);
    }
    
    // 打印SQL语句
    qDebug() << "SQL查询语句：" << queryStr;
    
    if(!query.exec()){
        qDebug() << "查询失败：" << query.lastError().text();
        return;
    }

    // 打印查询结果数量
    int count = 0;
    while(query.next()) {
        count++;
        int row = tablewidget->rowCount();
        tablewidget->insertRow(row);

        for (int col = 0; col < 6; col++) {
            QTableWidgetItem* item = new QTableWidgetItem(query.value(col).toString());
            item->setTextAlignment(Qt::AlignCenter);
            tablewidget->setItem(row, col, item);
        }
    }
    qDebug() << "查询结果数量：" << count;

    // 表头自适应
    tablewidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //   updateChart();
    //   updatePieChart();
}

//学生姓名下拉列表
void MoneyWidget::populateStudentComboBox()
{
    studentComboBox->clear();
    studentComboBox->addItem("所有学生", QVariant("-1")); //"-1"表示所有学生

    QSqlQuery query("SELECT student_id, name FROM student");
    while(query.next()) {
        QString id = query.value(0).toString(); //student_id 是字符串类型
        QString name = query.value(1).toString();
        studentComboBox->addItem(name, QVariant(id));
    }
}
//添加财务记录
void MoneyWidget::addRecord()
{
    QDialog dialog(this);
    dialog.setWindowTitle("添加缴费记录");
    QFormLayout form(&dialog);

    //学生名称下拉菜单
    QComboBox* studentComboBox = new QComboBox(&dialog);
    QSqlQuery query("SELECT student_id, name FROM student");
    while (query.next()) {
        QString id = query.value(0).toString();
        QString name = query.value(1).toString();
        studentComboBox->addItem(name, QVariant(id));//将学生ID与名称关联
    }
    QDateEdit* paymentDateEdit = new QDateEdit(&dialog);
    paymentDateEdit->setDate(QDate::currentDate());//设置默认值为当前日期
    paymentDateEdit->setCalendarPopup(true);//允许弹出日历选择器

    QLineEdit* amountEdit = new QLineEdit(&dialog);
    QLineEdit* feeTypeEdit = new QLineEdit(&dialog);
    QLineEdit* remarkEdit = new QLineEdit(&dialog);

    form.addRow("学生名称:", studentComboBox);
    form.addRow("缴费日期:", paymentDateEdit); //修改为QDateEdit
    form.addRow("金额", amountEdit);
    form.addRow("支付类型:", feeTypeEdit);
    form.addRow("备注:", remarkEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    buttonBox.button(QDialogButtonBox::Ok)->setText("确认");
    buttonBox.button(QDialogButtonBox::Cancel)->setText("取消");
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if(dialog.exec() == QDialog::Accepted) {
        QString studentId = studentComboBox->currentData().toString();
        QString paymentDate = paymentDateEdit->date().toString("yyyy-MM-dd");
        double amount = amountEdit->text().toDouble();
        QString feeType = feeTypeEdit->text();
        QString remark = remarkEdit->text();

        QSqlQuery query;
        query.prepare("INSERT INTO financialRecords (student_id, payment_date, amount, payment_type, notes)"
                      "VALUES(:student_id, :payment_date, :amount, :payment_type, :notes)");
        query.bindValue(":student_id", studentId); //绑定学生ID
        query.bindValue(":payment_date", paymentDate);
        query.bindValue(":amount", amount);
        query.bindValue(":payment_type", feeType);
        query.bindValue(":notes", remark);

        if(query.exec()) {
            qDebug() << "添加记录成功";
            locadMoney();
        }
        else {
            qDebug() << "添加记录失败" << query.lastError().text();
        }
    }


}

void MoneyWidget::editRecord()
{
  int currentRow = tablewidget->currentRow();
  if (currentRow < 0) {
     QMessageBox::warning(this, "警告", "请选择要修改的记录");
     return;
  }

  //获取当前行的数据
  QString id = tablewidget->item(currentRow, 0)->text(); //ID是字符串类型
  QString studentName = tablewidget->item(currentRow, 1)->text();
  QString paymentDate = tablewidget->item(currentRow, 2)->text();
  QString amount = tablewidget->item(currentRow, 3)->text();
  QString feeType = tablewidget->item(currentRow, 4)->text();
  QString remark = tablewidget->item(currentRow, 5)->text();
  QDialog dialog(this);
  dialog.setWindowTitle("修改缴费记录");
  QFormLayout form(&dialog);

  // 学生名称下拉框
  QComboBox* studentNameComboBox = new QComboBox(&dialog);
  QSqlQuery studentQuery("SELECT student_id, name FROM student");
  while (studentQuery.next()) {
      QString studentId = studentQuery.value(0).toString();
      QString name = studentQuery.value(1).toString();
      studentNameComboBox->addItem(name, QVariant(studentId));
  }
  studentNameComboBox->setCurrentText(studentName); //设置当前学生名称
  
  // 其他输入控件
  QDateEdit* paymentDateEdit = new QDateEdit(&dialog);
  paymentDateEdit->setCalendarPopup(true); // 启用日历弹出
  paymentDateEdit->setDate(QDate::fromString(paymentDate, "yyyy-MM-dd")); // 设置初始日期
  QLineEdit* amountEdit = new QLineEdit(amount, &dialog);
  QLineEdit* feeTypeEdit = new QLineEdit(feeType, &dialog);
  QLineEdit* remarkEdit = new QLineEdit(remark, &dialog);

  // 添加到表单
  form.addRow("学生名称:", studentNameComboBox);
  form.addRow("缴费日期:", paymentDateEdit);
  form.addRow("金额", amountEdit);
  form.addRow("支付类型:", feeTypeEdit);
  form.addRow("备注:", remarkEdit);

  // 按钮
  QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
  buttonBox.button(QDialogButtonBox::Ok)->setText("确认");
  buttonBox.button(QDialogButtonBox::Cancel)->setText("取消");
  form.addRow(&buttonBox);
  QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  if(dialog.exec() == QDialog::Accepted) {
      QString studentId = studentNameComboBox->currentData().toString();
      QString paymentDate = paymentDateEdit->date().toString("yyyy-MM-dd");
      double amount = amountEdit->text().toDouble();
      QString feeType = feeTypeEdit->text();
      QString remark = remarkEdit->text();

      //准备SQL查询
      QSqlQuery query;
      query.prepare("UPDATE financialRecords SET student_id = :student_id, payment_date = :payment_date, "
                    "amount = :amount, payment_type = :payment_type, notes = :notes WHERE id = :id");
      query.bindValue(":student_id", studentId);
      query.bindValue(":payment_date", paymentDate);
      query.bindValue(":amount", amount);
      query.bindValue(":payment_type", feeType);
      query.bindValue(":notes", remark);
      query.bindValue(":id", id);

      //执行查询
      if(query.exec()) {
          qDebug() << "记录修改成功";
          locadMoney();
      }
      else {
          qDebug() << "记录修改失败" << query.lastError().text();
      }
  }

}

void MoneyWidget::deleteRecord()
{
 
  int currentRow = tablewidget->currentRow(); if (currentRow < 0) {
     QMessageBox::warning(this, "警告", "请选择要删除的记录");
     return;
  }

  // 获取当前行的ID
  QString id = tablewidget->item(currentRow, 0)->text();
  QString studentName = tablewidget->item(currentRow, 1)->text();
  QString paymentDate = tablewidget->item(currentRow, 2)->text();

  // 显示确认对话框
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, "确认删除", 
                               QString("确定要删除 %1 在 %2 的缴费记录吗？").arg(studentName).arg(paymentDate),
                               QMessageBox::Yes | QMessageBox::No);

  if (reply == QMessageBox::Yes) {
      // 执行删除操作
      QSqlQuery query;
      query.prepare("DELETE FROM financialRecords WHERE id = :id");
      query.bindValue(":id", id);

      if(query.exec()) {
          qDebug() << "记录删除成功";
          locadMoney(); // 重新加载数据
      }
      else {
          qDebug() << "记录删除失败" << query.lastError().text();
          QMessageBox::critical(this, "错误", "删除失败：" + query.lastError().text());
      }
  }
}
