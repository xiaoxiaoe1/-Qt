#ifndef SCHEDULEWIDGET_H
#define SCHEDULEWIDGET_H

#include <QWidget>
#include <QMap>
#include <QDate>
#include <QComboBox>
#include <QTableWidget>

class QTableWidget;
class QComboBox;
class QLabel;
class QPushButton;
class handleItemChanged;
//防止头文件重复定义加上inline
inline int customWeekNumber(const QDate& date) {
//    QDate startOfYear(date.year(), 1, 1);
//    int dayOfWeek = startOfYear.dayOfWeek();
//    int daysToSubtract = startOfYear.dayOfWeek() - Qt::Monday;
//    if (daysToSubtract > 0) startOfYear = startOfYear.addDays(-daysToSubtract);
 //   int days = startOfYear.daysTo(date);
 //   int week = days / 7 + 1;
//    return week;
     return date.weekNumber(); // Qt内置精准周数计算
}

namespace Ui {
class schedulewidget;
}

class schedulewidget : public QWidget
{
    Q_OBJECT

public:
    explicit schedulewidget(QWidget *parent = nullptr);
    ~schedulewidget();

private:
    void setupUI(int currentYear, int currentWeek);
    void setupTable();
    void loadSchedule();
    void addCourse();
    void handleItemChanged(QTableWidgetItem* item);
    void deleteCourse();
    void showPreviousWeek();
    void showNextWeek();
    QPair<QDate, QDate> getWeekRange(int year, int week);
    QTableWidget* tablewidget;
    QComboBox* yearboBox;
    QComboBox* weekboBox;
    QLabel* dataRangeLabel; //显示日期范围标签
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* prevButton;
    QPushButton* nextButton;
    //课程数据存储结构：键为（year, week）,值为课程表数据
    QMap<QPair<int, int>, QVector<QVector<QString>>> scheduleData;
//    QStringList times;//上午1 上午2
    QStringList times = {"上午1", "上午2", "下午1", "下午2", "晚上1", "晚上2"};
    Ui::schedulewidget *ui;
};


#endif // SCHEDULEWIDGET_H
