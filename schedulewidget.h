#ifndef SCHEDULEWIDGET_H
#define SCHEDULEWIDGET_H

#include <QDate>
#include <QMap>
#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;

// 使用 Qt 内置的周数算法，确保与系统日期规则保持一致。
inline int customWeekNumber(const QDate &date)
{
    return date.weekNumber();
}

namespace Ui {
class schedulewidget;
}

// 课表页面提供按周查看、录入和删除课程安排的能力。
class schedulewidget : public QWidget
{
    Q_OBJECT

public:
    explicit schedulewidget(QWidget *parent = nullptr);
    ~schedulewidget() override;

private:
    void setupUI(int currentYear, int currentWeek);
    void setupTable();
    void loadSchedule();
    void addCourse();
    void handleItemChanged(QTableWidgetItem *item);
    void deleteCourse();
    void showPreviousWeek();
    void showNextWeek();
    QPair<QDate, QDate> getWeekRange(int year, int week);

    QTableWidget *tablewidget;
    QComboBox *yearboBox;
    QComboBox *weekboBox;
    QLabel *dataRangeLabel;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *prevButton;
    QPushButton *nextButton;

    // 保留缓存字段，后续如果要做本地周视图缓存可以直接复用。
    QMap<QPair<int, int>, QVector<QVector<QString>>> scheduleData;
    QStringList times = {"上午1", "上午2", "下午1", "下午2", "晚上1", "晚上2"};

    Ui::schedulewidget *ui;
};

#endif // SCHEDULEWIDGET_H
