#ifndef MONEYWIDGET_H
#define MONEYWIDGET_H

#include <QWidget>

//class QChartView;
// ✅【关键修改1】前向声明也要加上命名空间
namespace QtCharts {
    class QChartView;
}
class QTableWidget;
class QComboBox;
class QPushButton;
class QDateEdit;
namespace Ui {
class MoneyWidget;
}

class MoneyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MoneyWidget(QWidget *parent = nullptr);
    ~MoneyWidget();

private:
    void setupUI();
    void locadMoney();
    void populateStudentComboBox();
    void addRecord();
    void editRecord();
    void deleteRecord();
    QtCharts::QChartView* pieChartView;
    QtCharts::QChartView* chartView;
    QTableWidget* tablewidget;
    QComboBox* studentComboBox;
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* editButton;
    QDateEdit* startDateEdit;
    QDateEdit* endDateEdit;

    Ui::MoneyWidget *ui;
};

#endif // MONEYWIDGET_H
