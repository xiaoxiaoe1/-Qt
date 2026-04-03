#ifndef MONEYWIDGET_H
#define MONEYWIDGET_H

#include <QWidget>

namespace QtCharts {
class QChartView;
}

class QComboBox;
class QDateEdit;
class QPushButton;
class QTableWidget;

namespace Ui {
class MoneyWidget;
}

// 财务页面负责展示、筛选以及维护缴费记录。
class MoneyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MoneyWidget(QWidget *parent = nullptr);
    ~MoneyWidget() override;

private:
    void setupUI();
    void locadMoney();
    void populateStudentComboBox();
    void addRecord();
    void editRecord();
    void deleteRecord();

    QtCharts::QChartView *pieChartView;
    QtCharts::QChartView *chartView;
    QTableWidget *tablewidget;
    QComboBox *studentComboBox;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *editButton;
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;

    Ui::MoneyWidget *ui;
};

#endif // MONEYWIDGET_H
