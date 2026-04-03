#include "schedulewidget.h"
#include "ui_schedulewidget.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableWidget>
#include <QTime>
#include <QTimeEdit>
#include <QVBoxLayout>

schedulewidget::schedulewidget(QWidget *parent)
    : QWidget(parent)
    , tablewidget(nullptr)
    , yearboBox(nullptr)
    , weekboBox(nullptr)
    , dataRangeLabel(nullptr)
    , addButton(nullptr)
    , deleteButton(nullptr)
    , prevButton(nullptr)
    , nextButton(nullptr)
    , ui(new Ui::schedulewidget)
{
    ui->setupUi(this);

    const int currentYear = QDate::currentDate().year();
    const int currentWeek = customWeekNumber(QDate::currentDate());

    setupUI(currentYear, currentWeek);
    yearboBox->setCurrentText(QString::number(currentYear));
    weekboBox->setCurrentText(QString::fromUtf8("第 %1 周").arg(currentWeek));
    loadSchedule();
}

schedulewidget::~schedulewidget()
{
    delete ui;
}

void schedulewidget::setupUI(int currentYear, int currentWeek)
{
    Q_UNUSED(currentWeek);

    auto *mainLayout = new QVBoxLayout(this);
    auto *dateLayout = new QHBoxLayout();
    auto *buttonLayout = new QHBoxLayout();

    yearboBox = new QComboBox(this);
    weekboBox = new QComboBox(this);
    dataRangeLabel = new QLabel(this);

    for (int year = 2020; year <= currentYear + 5; ++year) {
        yearboBox->addItem(QString::number(year), year);
    }

    for (int week = 1; week <= 52; ++week) {
        weekboBox->addItem(QString::fromUtf8("第 %1 周").arg(week), week);
    }

    dateLayout->addWidget(new QLabel(QString::fromUtf8("年份："), this));
    dateLayout->addWidget(yearboBox);
    dateLayout->addWidget(new QLabel(QString::fromUtf8("周次："), this));
    dateLayout->addWidget(weekboBox);
    dateLayout->addWidget(dataRangeLabel);
    dateLayout->addStretch();

    tablewidget = new QTableWidget(this);
    tablewidget->setObjectName("scheduleTable");
    tablewidget->setAlternatingRowColors(true);
    tablewidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tablewidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    setupTable();

    prevButton = new QPushButton(QString::fromUtf8("上一周"), this);
    nextButton = new QPushButton(QString::fromUtf8("下一周"), this);
    addButton = new QPushButton(QString::fromUtf8("新增课程"), this);
    deleteButton = new QPushButton(QString::fromUtf8("删除课程"), this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);

    mainLayout->addLayout(dateLayout);
    mainLayout->addWidget(tablewidget);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    connect(yearboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &schedulewidget::loadSchedule);
    connect(weekboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &schedulewidget::loadSchedule);
    connect(addButton, &QPushButton::clicked, this, &schedulewidget::addCourse);
    connect(deleteButton, &QPushButton::clicked, this, &schedulewidget::deleteCourse);
    connect(prevButton, &QPushButton::clicked, this, &schedulewidget::showPreviousWeek);
    connect(nextButton, &QPushButton::clicked, this, &schedulewidget::showNextWeek);
    connect(tablewidget, &QTableWidget::itemChanged, this, &schedulewidget::handleItemChanged);
}

void schedulewidget::setupTable()
{
    const QStringList days = {
        QString::fromUtf8("周一"),
        QString::fromUtf8("周二"),
        QString::fromUtf8("周三"),
        QString::fromUtf8("周四"),
        QString::fromUtf8("周五"),
        QString::fromUtf8("周六"),
        QString::fromUtf8("周日")
    };

    tablewidget->setRowCount(days.count());
    tablewidget->setColumnCount(times.count());

    const int year = yearboBox->currentData().toInt();
    const int week = weekboBox->currentData().toInt();
    const QPair<QDate, QDate> weekRange = getWeekRange(year, week);
    const QDate startDate = weekRange.first;

    QStringList verticalHeaders;
    for (int i = 0; i < days.count(); ++i) {
        const QDate currentDate = startDate.addDays(i);
        verticalHeaders.append(QString("%1\n%2").arg(days[i], currentDate.toString("MM/dd")));
    }

    tablewidget->setVerticalHeaderLabels(verticalHeaders);
    tablewidget->setHorizontalHeaderLabels(times);
    tablewidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablewidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablewidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
}

void schedulewidget::loadSchedule()
{
    tablewidget->blockSignals(true);
    tablewidget->clearContents();

    const int year = yearboBox->currentData().toInt();
    const int week = weekboBox->currentData().toInt();
    const QPair<QDate, QDate> weekRange = getWeekRange(year, week);
    const QDate startDate = weekRange.first;
    const QDate endDate = weekRange.second;

    dataRangeLabel->setText(QString::fromUtf8("%1 至 %2")
                            .arg(startDate.toString("yyyy-MM-dd"), endDate.toString("yyyy-MM-dd")));
    setupTable();

    QVector<QVector<QString>> courses(7, QVector<QString>(times.count(), ""));

    QSqlQuery query;
    query.prepare("SELECT date, time, course_name FROM schedule WHERE date BETWEEN ? AND ?");
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));

    if (!query.exec()) {
        QMessageBox::warning(this, QString::fromUtf8("查询失败"), query.lastError().text());
        tablewidget->blockSignals(false);
        return;
    }

    while (query.next()) {
        const QDate date = QDate::fromString(query.value(0).toString(), "yyyy-MM-dd");
        const QString time = query.value(1).toString();
        const int dayIndex = startDate.daysTo(date);
        const int timeIndex = times.indexOf(time);

        if (dayIndex >= 0 && dayIndex < 7 && timeIndex >= 0 && timeIndex < times.count()) {
            courses[dayIndex][timeIndex] = query.value(2).toString();
        }
    }

    for (int day = 0; day < 7; ++day) {
        for (int time = 0; time < times.count(); ++time) {
            auto *item = new QTableWidgetItem(courses[day][time]);
            item->setTextAlignment(Qt::AlignCenter);
            tablewidget->setItem(day, time, item);
        }
    }

    tablewidget->blockSignals(false);
}

void schedulewidget::addCourse()
{
    const int dayIndex = tablewidget->currentRow();
    const int timeIndex = tablewidget->currentColumn();

    if (dayIndex < 0 || timeIndex < 0) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先选择一个课表单元格。"));
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("新增课程"));
    QFormLayout layout(&dialog);

    QComboBox studentNameCombo;
    QSqlQuery studentQuery("SELECT name FROM student ORDER BY name");
    while (studentQuery.next()) {
        studentNameCombo.addItem(studentQuery.value(0).toString());
    }

    const QMap<int, QTime> timePresets = {
        {0, QTime(9, 0)},
        {1, QTime(11, 0)},
        {2, QTime(14, 0)},
        {3, QTime(16, 0)},
        {4, QTime(19, 0)},
        {5, QTime(21, 0)}
    };

    QTimeEdit timeEdit;
    timeEdit.setDisplayFormat("HH:mm");
    timeEdit.setTime(timePresets.value(timeIndex, QTime(9, 0)));

    layout.addRow(QString::fromUtf8("学生："), &studentNameCombo);
    layout.addRow(QString::fromUtf8("时间："), &timeEdit);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons.button(QDialogButtonBox::Ok)->setText(QString::fromUtf8("确认"));
    buttons.button(QDialogButtonBox::Cancel)->setText(QString::fromUtf8("取消"));
    layout.addRow(&buttons);

    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const int year = yearboBox->currentData().toInt();
    const int week = weekboBox->currentData().toInt();
    const QPair<QDate, QDate> weekRange = getWeekRange(year, week);
    const QDate currentDate = weekRange.first.addDays(dayIndex);
    const QString timeSlot = times[timeIndex];
    const QString courseName = QString("%1, %2")
            .arg(studentNameCombo.currentText(), timeEdit.time().toString("HH:mm"));

    QSqlQuery query;
    query.prepare("INSERT INTO schedule(date, time, course_name) VALUES(?, ?, ?)");
    query.addBindValue(currentDate.toString("yyyy-MM-dd"));
    query.addBindValue(timeSlot);
    query.addBindValue(courseName);

    if (!query.exec()) {
        QMessageBox::critical(this, QString::fromUtf8("新增失败"), query.lastError().text());
        return;
    }

    loadSchedule();
}

void schedulewidget::handleItemChanged(QTableWidgetItem *item)
{
    if (!item) {
        return;
    }

    const int day = item->row();
    const int timeSlot = item->column();
    if (day < 0 || day >= 7 || timeSlot < 0 || timeSlot >= times.count()) {
        return;
    }

    const QString newCourse = item->text().trimmed();
    const int year = yearboBox->currentData().toInt();
    const int week = weekboBox->currentData().toInt();
    const QPair<QDate, QDate> weekRange = getWeekRange(year, week);
    const QString dateStr = weekRange.first.addDays(day).toString("yyyy-MM-dd");
    const QString timeStr = times[timeSlot];

    tablewidget->blockSignals(true);

    QSqlQuery query;
    if (newCourse.isEmpty()) {
        query.prepare("DELETE FROM schedule WHERE date = ? AND time = ?");
        query.addBindValue(dateStr);
        query.addBindValue(timeStr);
    } else {
        query.prepare("INSERT OR REPLACE INTO schedule(date, time, course_name) VALUES(?, ?, ?)");
        query.addBindValue(dateStr);
        query.addBindValue(timeStr);
        query.addBindValue(newCourse);
    }

    if (!query.exec()) {
        QMessageBox::warning(this, QString::fromUtf8("保存失败"), query.lastError().text());
    }

    tablewidget->blockSignals(false);
}

void schedulewidget::deleteCourse()
{
    const int dayIndex = tablewidget->currentRow();
    const int timeIndex = tablewidget->currentColumn();

    if (dayIndex < 0 || timeIndex < 0) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先选择一条课程记录。"));
        return;
    }

    auto *item = tablewidget->item(dayIndex, timeIndex);
    if (!item || item->text().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("当前单元格没有课程。"));
        return;
    }

    const auto reply = QMessageBox::question(this,
                                             QString::fromUtf8("确认删除"),
                                             QString::fromUtf8("确认删除当前课程吗？"),
                                             QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }

    const int year = yearboBox->currentData().toInt();
    const int week = weekboBox->currentData().toInt();
    const QPair<QDate, QDate> weekRange = getWeekRange(year, week);
    const QDate currentDate = weekRange.first.addDays(dayIndex);

    QSqlQuery query;
    query.prepare("DELETE FROM schedule WHERE date = ? AND time = ?");
    query.addBindValue(currentDate.toString("yyyy-MM-dd"));
    query.addBindValue(times[timeIndex]);

    if (!query.exec()) {
        QMessageBox::critical(this, QString::fromUtf8("删除失败"), query.lastError().text());
        return;
    }

    loadSchedule();
}

void schedulewidget::showPreviousWeek()
{
    const int currentWeek = weekboBox->currentIndex();
    const int currentYear = yearboBox->currentIndex();

    if (currentWeek > 0) {
        weekboBox->setCurrentIndex(currentWeek - 1);
        return;
    }

    if (currentYear > 0) {
        yearboBox->setCurrentIndex(currentYear - 1);
        weekboBox->setCurrentIndex(51);
    }
}

void schedulewidget::showNextWeek()
{
    const int currentWeek = weekboBox->currentIndex();
    const int currentYear = yearboBox->currentIndex();

    if (currentWeek < weekboBox->count() - 1) {
        weekboBox->setCurrentIndex(currentWeek + 1);
        return;
    }

    if (currentYear < yearboBox->count() - 1) {
        yearboBox->setCurrentIndex(currentYear + 1);
        weekboBox->setCurrentIndex(0);
    }
}

QPair<QDate, QDate> schedulewidget::getWeekRange(int year, int week)
{
    // Match Qt's weekNumber() behavior: find a date inside the target week,
    // then walk back to Monday.
    QDate anyDateInWeek(year, 1, 4);
    while (anyDateInWeek.weekNumber() != week) {
        anyDateInWeek = anyDateInWeek.addDays(7);
        if (anyDateInWeek.year() > year + 1) {
            break;
        }
    }

    const QDate weekStart = anyDateInWeek.addDays(1 - anyDateInWeek.dayOfWeek());
    const QDate weekEnd = weekStart.addDays(6);

    return qMakePair(weekStart, weekEnd);
}
