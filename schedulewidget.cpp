#include "schedulewidget.h"
#include "ui_schedulewidget.h"
#include <QTabWidget>
#include <QComboBox>
#include <QHeaderView>
#include <QDate>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>
#include <QFormLayout>
#include <QTimeEdit>
#include <QTime>
#include <QDialogButtonBox> //标准按钮盒子里面包括确定、取消、应用、关闭 那一排按钮。
schedulewidget::schedulewidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::schedulewidget)
{
    ui->setupUi(this);
    // 【第一步：先计算当前年周，提前初始化】
    int currentYear = QDate::currentDate().year();
    int currentWeek = customWeekNumber(QDate::currentDate());
    // 【第二步：初始化UI，传入当前年周】
    setupUI(currentYear, currentWeek);
    // 【第三步：设置下拉框选中当前年周】
    yearboBox->setCurrentText(QString::number(currentYear));
    weekboBox->setCurrentText(QString("第 %1 周").arg(currentWeek));
    // 【第四步：加载数据】
    loadSchedule();
}

schedulewidget::~schedulewidget()
{
    delete ui;
}

void schedulewidget::setupUI(int currentYear, int currentWeek)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* dateLayout = new QHBoxLayout();
    yearboBox = new QComboBox(this);
    weekboBox = new QComboBox(this);

    //    int currentYear = QDate::currentDate().year();
    // 初始化年份下拉框
    for (int year = 2020; year <= currentYear + 5; ++year)
        yearboBox->addItem(QString::number(year), year);
    // 初始化周数下拉框
    for (int week = 1; week <= 52; ++week)
        weekboBox->addItem(QString("第 %1 周").arg(week), week);

    dataRangeLabel = new QLabel(this);
    // 添加周导航按钮
    QPushButton* prevWeekBtn = new QPushButton("上一周", this);
    QPushButton* nextWeekBtn = new QPushButton("下一周", this);
    prevWeekBtn->setFixedWidth(200);
    nextWeekBtn->setFixedWidth(200);

    dateLayout->addWidget(new QLabel("年份：", this));
    dateLayout->addWidget(yearboBox);
    dateLayout->addWidget(new QLabel("周数：", this));
    dateLayout->addWidget(weekboBox);
    dateLayout->addWidget(dataRangeLabel);
    dateLayout->addStretch();

    tablewidget = new QTableWidget(this);
    tablewidget->setAlternatingRowColors(true);
    tablewidget->setSelectionMode(QAbstractItemView::SingleSelection);
    // 年周已就绪，setupTable能拿到正确日期
    setupTable();

    addButton = new QPushButton("添加课程", this);
    deleteButton = new QPushButton("删除课程", this);
    addButton->setFixedWidth(200);
    deleteButton->setFixedWidth(200);

    //    connect(yearboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //            this, &ScheduleWidget::loadSchedule);
    //    connect(weekboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //            this, &ScheduleWidget::loadSchedule);
    connect(addButton, &QPushButton::clicked, this, &schedulewidget::addCourse);
    connect(deleteButton, &QPushButton::clicked, this, &schedulewidget::deleteCourse);
    connect(prevWeekBtn, &QPushButton::clicked, this, &schedulewidget::showPreviousWeek);
    connect(nextWeekBtn, &QPushButton::clicked, this, &schedulewidget::showNextWeek);
    connect(tablewidget, &QTableWidget::itemChanged, this, &schedulewidget::handleItemChanged);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(prevWeekBtn);
    buttonLayout->addWidget(nextWeekBtn);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(dateLayout);
    mainLayout->addWidget(tablewidget);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

}

void schedulewidget::setupTable()
{
    QStringList days = {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};
    //问题是在C++中，QStringList不能直接使用花括号初始化列表进行赋值，需要使用QStringList() << 的方式来初始化。
    //    times = QStringList() << "上午1" << "上午2" << "下午1" << "下午2" << "晚上1" << "晚上2";
    tablewidget->setRowCount(days.count());
    tablewidget->setColumnCount(times.count());
    int year = yearboBox->currentData().toInt();
    int week = weekboBox->currentData().toInt();
    QPair<QDate, QDate> weekRange = getWeekRange(year, week);
    QDate startDate = weekRange.first;
    QStringList verticalHeaders;
    for (int i = 0; i < days.count(); ++i) {
        QDate currentDate = startDate.addDays(i);
        verticalHeaders.append(QString("%1\n%2").arg(days[i]).arg(currentDate.toString("MM/dd")));
    }
    tablewidget->setVerticalHeaderLabels(verticalHeaders);
    tablewidget->setHorizontalHeaderLabels(times);
    tablewidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablewidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablewidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
}

void schedulewidget::loadSchedule()
{
    // 先检查数据库连接
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "❌ loadSchedule失败：数据库未连接";
        return;
    }
    qDebug() << "✅ 数据库连接正常，开始加载课表";
    tablewidget->blockSignals(true); //防止加载数据触发itemChanged信号
    tablewidget->clearContents();

    int year = yearboBox->currentData().toInt();
    int week = weekboBox->currentData().toInt();
    QPair<QDate, QDate> weekRange = getWeekRange(year, week);
    QDate startDate = weekRange.first;
    QDate endDate = weekRange.second;

    // 打印查询范围，方便调试
    qDebug() << "查询日期范围：" << startDate.toString("yyyy-MM-dd") << " 到 " << endDate.toString("yyyy-MM-dd");


    dataRangeLabel->setText(startDate.toString("yyyy-MM-dd") + "到" + endDate.toString("yyyy-MM-dd"));

    // 同步更新表格行标题，确保日期匹配
    setupTable();

    QVector<QVector<QString>> courses(7, QVector<QString>(times.count(), ""));
    QSqlQuery query;
    query.prepare("SELECT date, time, course_name FROM schedule WHERE date BETWEEN ? AND ?");
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));

    if(!query.exec()) {
        qDebug() << "❌ 课表查询失败：" << query.lastError().text();
        tablewidget->blockSignals(false);
        return;
    }

    if(query.exec()) {
        while (query.next()) {
            QDate date = QDate::fromString(query.value(0).toString(), "yyyy-MM-dd");
            QString time = query.value(1).toString();
            int dayIndex = startDate.daysTo(date);
            int timeIndex = times.indexOf(time);
            if(dayIndex >= 0 && dayIndex < 7 && timeIndex != -1) {
                courses[dayIndex][timeIndex] = query.value(2).toString();
            }

        }
    }

    for(int day = 0; day < 7; ++day) {
        for (int time = 0; time < times.count(); ++time) {
            QTableWidgetItem* item = new QTableWidgetItem(courses[day][time]);
            item->setTextAlignment(Qt::AlignCenter);
            tablewidget->setItem(day, time, item);
        }
    }
    tablewidget->blockSignals(false);
}

void schedulewidget::addCourse()
{
    int dayIndex = tablewidget->currentRow();
    int timeIndex = tablewidget->currentColumn();

    if(dayIndex == -1 || timeIndex == -1) { //验证选中位置的有效性
        QMessageBox::warning(this, "错误", "请先选择一个单元格");
        return;
    }
    QDialog dialog(this); //创建自定义对话框
    dialog.setWindowTitle("添加课程");
    QFormLayout layout(&dialog);
    QComboBox nameCombo; //学生姓名下拉框
    QSqlQuery nameQuery("SELECT name FROM student");
    while (nameQuery.next()) nameCombo.addItem(nameQuery.value(0).toString());
    //时间映射表：列索引-> 默认时间
    QMap<int, QTime> timePresets = {//上午1到晚上2
                                    {0, QTime(9,0)},{1, QTime(11, 0)},{2, QTime(14, 0)},
                                    {3, QTime(16, 0)},{4, QTime(19, 0)},{5, QTime(21, 0)}
                                   };

    QTimeEdit timeEidt; //时间选择控件
    timeEidt.setDisplayFormat("HH-mm");
    timeEidt.setTime(timePresets.value(timeIndex));//设置列对应默认时间
    layout.addRow("学生姓名", &nameCombo);
    layout.addRow("课程时间", &timeEidt);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons.button(QDialogButtonBox::Ok)->setText("确认");
    buttons.button(QDialogButtonBox::Cancel)->setText("取消");
    layout.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if(dialog.exec() != QDialog::Accepted) return;
    //生成组合字符串
    QString courseName = QString("%1, %2").arg(nameCombo.currentText())
            .arg(timeEidt.time().toString("HH:mm"));
    //获取日期时间
    int year = yearboBox->currentData().toInt();
    int week = weekboBox->currentData().toInt();
    QPair<QDate, QDate> weekRange = getWeekRange(year, week);
    QDate currentDate = weekRange.first.addDays(dayIndex);
    QString timeSlot = times[timeIndex];//原始时间段标识

    //数据库操作
    QSqlQuery query;
    query.prepare("INSERT INTO schedule(date, time, course_name) VALUES(?, ?, ?)");
    query.addBindValue(currentDate.toString("yyyy-MM-dd"));
    query.addBindValue(timeSlot);//存储原始时间段
    query.addBindValue(courseName); //存储“姓名， HH：mm”格式

    if(!query.exec())
        QMessageBox::critical(this, "错误", "添加失败:" + query.lastError().text());
    else loadSchedule(); //刷新显示
}

void schedulewidget::handleItemChanged(QTableWidgetItem* item)
{
    // 【第一步：基础校验，防止空指针和越界】
    if(!item) return;
    int day = item->row();
    int timeSlot = item->column();
    if (day < 0 || day >= 7 || timeSlot < 0 || timeSlot >= times.count()) return;

    //【第二步：获取基础数据】
    QString newCourse = item->text().trimmed();
    int year = yearboBox->currentData().toInt();
    int week = weekboBox->currentData().toInt();
    QPair <QDate, QDate> weekRange = getWeekRange(year, week);
    QDate date = weekRange.first.addDays(day);
    QString dateStr = date.toString("yyyy-MM-dd"); // 统一日期格式，避免重复写错
    QString timeStr = times[timeSlot];

    // 【第三步：数据库操作，先屏蔽信号，防止递归】
    tablewidget->blockSignals(true); // 关键：屏蔽信号，避免操作过程中再次触发itemChanged
    QSqlQuery query;

    if(newCourse.isEmpty()) {
        //删除课程
        query.prepare("DELETE FROM schedule WHERE date = ? AND time = ?");
        query.addBindValue(dateStr);
        query.addBindValue(dateStr);
        query.addBindValue(timeStr);
    }
    else {
        //使用REPLACE语句更新或插入
        query.prepare("INSERT OR REPLACE INTO schedule (date, time, course_name) VALUES (?, ?, ?)");
        query.addBindValue(dateStr);
        query.addBindValue(timeStr);
        query.addBindValue(newCourse);
    }

    // 【第四步：执行结果处理】
    if(!query.exec()) {
        QMessageBox::warning(this, "错误", "操作失败:" + query.lastError().text());
        // 出错回滚单元格内容，不要调用loadSchedule，避免递归
        item->setText(tablewidget->item(day, timeSlot)->text());
    }
    tablewidget->blockSignals(false);//回复信号
}
//删除课程表的某条记录
void schedulewidget::deleteCourse()
{
    //确认删除按钮
    QMessageBox confirmBox(this);
    confirmBox.setWindowTitle("确认删除");
    confirmBox.setText("确认要删除改条记录吗?");
    //设置按钮为中文
    QPushButton* yesButton = confirmBox.addButton("确认", QMessageBox::YesRole);
    QPushButton* noButton = confirmBox.addButton("取消", QMessageBox::NoRole);

    //设置默认按钮
    confirmBox.setDefaultButton(noButton);

    //显示对话框并等待用户选择
    confirmBox.exec();

    if(confirmBox.clickedButton() == yesButton) {
        int dayIndex = tablewidget->currentRow();
        int timeIndex = tablewidget->currentColumn();

        if(dayIndex == -1 || timeIndex == -1) {
            QMessageBox::warning(this, "错误", "请选择一个时间段!");
            return;
        }
        //QTableWidgetItem 是表格里的一个小格子
        QTableWidgetItem*  item = tablewidget->item(dayIndex, timeIndex);
        if(!item || item->text().isEmpty()) {
            QMessageBox::warning(this, "错误", "该时间段没有课程!");
            return;
        }

        int year = yearboBox->currentData().toInt();
        int week = weekboBox->currentData().toInt();

        QPair<QDate, QDate> weekRange = getWeekRange(year, week);
        QDate currentDate = weekRange.first.addDays(dayIndex);
        QString time = times[timeIndex];

        QSqlQuery query;
        query.prepare("DELETE FROM schedule WHERE date = ? AND time = ?");
        query.addBindValue(currentDate.toString("yyyy-MM-dd"));
        query.addBindValue(time);

        if(!query.exec()) {
            QMessageBox::critical(this, "错误", "删除失败:" + query.lastError().text());
        }
        else {
            loadSchedule();
        }
    }
}
//上一周
void schedulewidget::showPreviousWeek()
{
    int currentWeek = weekboBox->currentIndex();
    int currentYear = yearboBox->currentIndex();

    if(currentWeek > 0) {
        weekboBox->setCurrentIndex(currentWeek - 1);
    }
    else {
        if(yearboBox->currentIndex() > 0) {
            yearboBox->setCurrentIndex(currentYear - 1);
            //跳转到上一年的最后一周（52周）
            weekboBox->setCurrentIndex(51);
        }
    }
}
//下一周
void schedulewidget::showNextWeek()
{
    int currentWeek = weekboBox->currentIndex();
    int currentYear = yearboBox->currentIndex();

    if(currentWeek < 51) {
        weekboBox->setCurrentIndex(currentWeek + 1);
    }
    else {
        if(yearboBox->currentIndex() < yearboBox->count()-1) {
            yearboBox->setCurrentIndex(currentYear + 1);
            weekboBox->setCurrentIndex(0);
        }

    }
}

QPair<QDate, QDate> schedulewidget::getWeekRange(int year, int week) {
    //    QDate startDate(year, 1, 1);
    //    int daysToSubtract = startDate.dayOfWeek() - Qt::Monday;
    //    if (daysToSubtract > 0) startDate = startDate.addDays(-daysToSubtract);
    //    QDate weekStart = startDate.addDays((week - 1) * 7);
    //    QDate weekEnd = weekStart.addDays(6);
    //    return qMakePair(weekStart, weekEnd);
    QDate weekStart;
    weekStart.setDate(year, 1, 1);
    // 调整到该年第一个周一
    if(weekStart.dayOfWeek() > Qt::Monday) {
        weekStart = weekStart.addDays(8 - weekStart.dayOfWeek());
    } else if(weekStart.dayOfWeek() < Qt::Monday) {
        weekStart = weekStart.addDays(Qt::Monday - weekStart.dayOfWeek());
    }
    // 计算目标周的周一
    weekStart = weekStart.addDays((week - 1) * 7);
    QDate weekEnd = weekStart.addDays(6);
    return qMakePair(weekStart, weekEnd);
}
