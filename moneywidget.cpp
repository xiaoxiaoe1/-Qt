#include "moneywidget.h"
#include "ui_moneywidget.h"

#include <QChart>
#include <QChartView>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableWidget>
#include <QVBoxLayout>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>

using namespace QtCharts;

MoneyWidget::MoneyWidget(QWidget *parent)
    : QWidget(parent)
    , pieChartView(nullptr)
    , chartView(nullptr)
    , tablewidget(nullptr)
    , studentComboBox(nullptr)
    , addButton(nullptr)
    , deleteButton(nullptr)
    , editButton(nullptr)
    , startDateEdit(nullptr)
    , endDateEdit(nullptr)
    , ui(new Ui::MoneyWidget)
{
    ui->setupUi(this);
    setupUI();
    populateStudentComboBox();
    locadMoney();
}

MoneyWidget::~MoneyWidget()
{
    delete ui;
}

void MoneyWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *filterLayout = new QHBoxLayout();
    auto *middleLayout = new QHBoxLayout();

    studentComboBox = new QComboBox(this);
    studentComboBox->setObjectName("moneyStudentFilter");

    startDateEdit = new QDateEdit(QDate::currentDate().addMonths(-1), this);
    startDateEdit->setCalendarPopup(true);

    endDateEdit = new QDateEdit(QDate::currentDate(), this);
    endDateEdit->setCalendarPopup(true);

    addButton = new QPushButton(QString::fromUtf8("新增记录"), this);
    editButton = new QPushButton(QString::fromUtf8("修改记录"), this);
    deleteButton = new QPushButton(QString::fromUtf8("删除记录"), this);

    filterLayout->addWidget(new QLabel(QString::fromUtf8("学生："), this));
    filterLayout->addWidget(studentComboBox);
    filterLayout->addWidget(new QLabel(QString::fromUtf8("开始日期："), this));
    filterLayout->addWidget(startDateEdit);
    filterLayout->addWidget(new QLabel(QString::fromUtf8("结束日期："), this));
    filterLayout->addWidget(endDateEdit);
    filterLayout->addWidget(addButton);
    filterLayout->addWidget(editButton);
    filterLayout->addWidget(deleteButton);
    filterLayout->addStretch();

    tablewidget = new QTableWidget(this);
    tablewidget->setObjectName("financeTable");
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setAlternatingRowColors(true);
    tablewidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablewidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tablewidget->setMinimumHeight(420);

    const QStringList headers = {
        QString::fromUtf8("编号"),
        QString::fromUtf8("学生"),
        QString::fromUtf8("缴费日期"),
        QString::fromUtf8("金额"),
        QString::fromUtf8("缴费类型"),
        QString::fromUtf8("备注")
    };
    tablewidget->setColumnCount(headers.count());
    tablewidget->setHorizontalHeaderLabels(headers);
    tablewidget->setColumnHidden(0, true);
    tablewidget->horizontalHeader()->setStretchLastSection(true);

    pieChartView = new QChartView(this);
    pieChartView->setMinimumWidth(320);
    pieChartView->setRenderHint(QPainter::Antialiasing);
    pieChartView->setChart(new QChart());
    pieChartView->chart()->setTitle(QString::fromUtf8("缴费结构"));

    chartView = new QChartView(this);
    chartView->setMinimumHeight(220);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setChart(new QChart());
    chartView->chart()->setTitle(QString::fromUtf8("近期缴费趋势"));

    middleLayout->addWidget(tablewidget, 3);
    middleLayout->addWidget(pieChartView, 2);

    mainLayout->addLayout(filterLayout);
    mainLayout->addLayout(middleLayout, 1);
    mainLayout->addWidget(chartView);

    connect(addButton, &QPushButton::clicked, this, &MoneyWidget::addRecord);
    connect(editButton, &QPushButton::clicked, this, &MoneyWidget::editRecord);
    connect(deleteButton, &QPushButton::clicked, this, &MoneyWidget::deleteRecord);
    connect(studentComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MoneyWidget::locadMoney);
    connect(startDateEdit, &QDateEdit::dateChanged, this, &MoneyWidget::locadMoney);
    connect(endDateEdit, &QDateEdit::dateChanged, this, &MoneyWidget::locadMoney);
}

void MoneyWidget::locadMoney()
{
    tablewidget->setRowCount(0);

    const QString studentId = studentComboBox->currentData().toString();
    const QString startDate = startDateEdit->date().toString("yyyy-MM-dd");
    const QString endDate = endDateEdit->date().toString("yyyy-MM-dd");

    QString queryText =
            "SELECT fr.id, s.name, fr.payment_date, fr.amount, fr.payment_type, fr.notes "
            "FROM financialRecords fr "
            "JOIN student s ON fr.student_id = s.student_id "
            "WHERE fr.payment_date BETWEEN ? AND ? ";

    if (studentId != "-1") {
        queryText += "AND fr.student_id = ? ";
    }

    queryText += "ORDER BY fr.payment_date DESC, fr.id DESC";

    QSqlQuery query;
    query.prepare(queryText);
    query.addBindValue(startDate);
    query.addBindValue(endDate);
    if (studentId != "-1") {
        query.addBindValue(studentId);
    }

    if (!query.exec()) {
        QMessageBox::warning(this, QString::fromUtf8("查询失败"), query.lastError().text());
        return;
    }

    while (query.next()) {
        const int row = tablewidget->rowCount();
        tablewidget->insertRow(row);

        for (int col = 0; col < tablewidget->columnCount(); ++col) {
            auto *item = new QTableWidgetItem(query.value(col).toString());
            item->setTextAlignment(Qt::AlignCenter);
            tablewidget->setItem(row, col, item);
        }
    }

    tablewidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MoneyWidget::populateStudentComboBox()
{
    studentComboBox->clear();
    studentComboBox->addItem(QString::fromUtf8("全部学生"), "-1");

    QSqlQuery query("SELECT student_id, name FROM student ORDER BY name");
    while (query.next()) {
        studentComboBox->addItem(query.value(1).toString(), query.value(0).toString());
    }
}

void MoneyWidget::addRecord()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("新增缴费记录"));

    QFormLayout form(&dialog);

    auto *studentSelector = new QComboBox(&dialog);
    QSqlQuery studentQuery("SELECT student_id, name FROM student ORDER BY name");
    while (studentQuery.next()) {
        studentSelector->addItem(studentQuery.value(1).toString(), studentQuery.value(0).toString());
    }

    auto *paymentDateEdit = new QDateEdit(QDate::currentDate(), &dialog);
    paymentDateEdit->setCalendarPopup(true);

    auto *amountEdit = new QLineEdit(&dialog);
    auto *paymentTypeEdit = new QLineEdit(&dialog);
    auto *remarkEdit = new QLineEdit(&dialog);

    form.addRow(QString::fromUtf8("学生："), studentSelector);
    form.addRow(QString::fromUtf8("缴费日期："), paymentDateEdit);
    form.addRow(QString::fromUtf8("金额："), amountEdit);
    form.addRow(QString::fromUtf8("缴费类型："), paymentTypeEdit);
    form.addRow(QString::fromUtf8("备注："), remarkEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    buttonBox.button(QDialogButtonBox::Ok)->setText(QString::fromUtf8("确认"));
    buttonBox.button(QDialogButtonBox::Cancel)->setText(QString::fromUtf8("取消"));
    form.addRow(&buttonBox);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QSqlQuery query;
    query.prepare(
                "INSERT INTO financialRecords (student_id, payment_date, amount, payment_type, notes) "
                "VALUES (:student_id, :payment_date, :amount, :payment_type, :notes)");
    query.bindValue(":student_id", studentSelector->currentData().toString());
    query.bindValue(":payment_date", paymentDateEdit->date().toString("yyyy-MM-dd"));
    query.bindValue(":amount", amountEdit->text().toDouble());
    query.bindValue(":payment_type", paymentTypeEdit->text().trimmed());
    query.bindValue(":notes", remarkEdit->text().trimmed());

    if (!query.exec()) {
        QMessageBox::warning(this, QString::fromUtf8("新增失败"), query.lastError().text());
        return;
    }

    locadMoney();
}

void MoneyWidget::editRecord()
{
    const int currentRow = tablewidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先选择要修改的记录。"));
        return;
    }

    const QString id = tablewidget->item(currentRow, 0)->text();
    const QString studentName = tablewidget->item(currentRow, 1)->text();
    const QString paymentDate = tablewidget->item(currentRow, 2)->text();
    const QString amount = tablewidget->item(currentRow, 3)->text();
    const QString feeType = tablewidget->item(currentRow, 4)->text();
    const QString remark = tablewidget->item(currentRow, 5)->text();

    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("修改缴费记录"));
    QFormLayout form(&dialog);

    auto *studentNameComboBox = new QComboBox(&dialog);
    QSqlQuery studentQuery("SELECT student_id, name FROM student ORDER BY name");
    while (studentQuery.next()) {
        studentNameComboBox->addItem(studentQuery.value(1).toString(), studentQuery.value(0).toString());
    }
    studentNameComboBox->setCurrentText(studentName);

    auto *paymentDateEdit = new QDateEdit(&dialog);
    paymentDateEdit->setCalendarPopup(true);
    paymentDateEdit->setDate(QDate::fromString(paymentDate, "yyyy-MM-dd"));

    auto *amountEdit = new QLineEdit(amount, &dialog);
    auto *feeTypeEdit = new QLineEdit(feeType, &dialog);
    auto *remarkEdit = new QLineEdit(remark, &dialog);

    form.addRow(QString::fromUtf8("学生："), studentNameComboBox);
    form.addRow(QString::fromUtf8("缴费日期："), paymentDateEdit);
    form.addRow(QString::fromUtf8("金额："), amountEdit);
    form.addRow(QString::fromUtf8("缴费类型："), feeTypeEdit);
    form.addRow(QString::fromUtf8("备注："), remarkEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    buttonBox.button(QDialogButtonBox::Ok)->setText(QString::fromUtf8("确认"));
    buttonBox.button(QDialogButtonBox::Cancel)->setText(QString::fromUtf8("取消"));
    form.addRow(&buttonBox);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QSqlQuery query;
    query.prepare(
                "UPDATE financialRecords "
                "SET student_id = :student_id, payment_date = :payment_date, amount = :amount, "
                "payment_type = :payment_type, notes = :notes "
                "WHERE id = :id");
    query.bindValue(":student_id", studentNameComboBox->currentData().toString());
    query.bindValue(":payment_date", paymentDateEdit->date().toString("yyyy-MM-dd"));
    query.bindValue(":amount", amountEdit->text().toDouble());
    query.bindValue(":payment_type", feeTypeEdit->text().trimmed());
    query.bindValue(":notes", remarkEdit->text().trimmed());
    query.bindValue(":id", id);

    if (!query.exec()) {
        QMessageBox::warning(this, QString::fromUtf8("修改失败"), query.lastError().text());
        return;
    }

    locadMoney();
}

void MoneyWidget::deleteRecord()
{
    const int currentRow = tablewidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先选择要删除的记录。"));
        return;
    }

    const QString id = tablewidget->item(currentRow, 0)->text();
    const QString studentName = tablewidget->item(currentRow, 1)->text();
    const QString paymentDate = tablewidget->item(currentRow, 2)->text();

    const auto reply = QMessageBox::question(
                this,
                QString::fromUtf8("确认删除"),
                QString::fromUtf8("确认删除 %1 在 %2 的缴费记录吗？").arg(studentName, paymentDate),
                QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM financialRecords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        QMessageBox::critical(this, QString::fromUtf8("删除失败"), query.lastError().text());
        return;
    }

    locadMoney();
}
