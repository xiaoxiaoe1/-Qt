#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "studeninfowight.h"

#include <QButtonGroup>
#include <QDebug>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    initializeDatabase();

    // Build widgets after the database is ready.
    ui->setupUi(this);

    initializeWindowText();
    initializeNavigation();
    loadStudentData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeDatabase()
{
    // Reuse the default connection if it already exists.
    QSqlDatabase db = QSqlDatabase::contains()
            ? QSqlDatabase::database()
            : QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName("student.db");

    if (!db.open()) {
        const QString errorText = db.lastError().text();
        qDebug() << "Failed to open database:" << errorText;
        QMessageBox::critical(this, "Database Error", "Cannot open student.db:\n" + errorText);
        return;
    }

    QSqlQuery query(db);

    if (!query.exec(
                "CREATE TABLE IF NOT EXISTS student ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "student_id TEXT UNIQUE,"
                "name TEXT NOT NULL,"
                "gender TEXT,"
                "age INTEGER,"
                "phone TEXT,"
                "address TEXT,"
                "class_name TEXT,"
                "avatar BLOB"
                ")")) {
        qDebug() << "Create student failed:" << query.lastError().text();
    }

    if (!query.exec(
                "CREATE TABLE IF NOT EXISTS schedule ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "date TEXT NOT NULL,"
                "time TEXT NOT NULL,"
                "course_name TEXT NOT NULL,"
                "UNIQUE(date, time) ON CONFLICT REPLACE"
                ")")) {
        qDebug() << "Create schedule failed:" << query.lastError().text();
    }

    if (!query.exec(
                "CREATE TABLE IF NOT EXISTS financialRecords ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "student_id TEXT,"
                "payment_date TEXT,"
                "amount REAL,"
                "payment_type TEXT,"
                "notes TEXT,"
                "FOREIGN KEY (student_id) REFERENCES student(student_id)"
                ")")) {
        qDebug() << "Create financialRecords failed:" << query.lastError().text();
    }
}

void MainWindow::initializeNavigation()
{
    // These object names let QSS style the nav and content areas precisely.
    ui->widget->setObjectName("leftNav");
    ui->stackedWidget->setObjectName("contentPanel");

    auto *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->tbnstdInfo, 0);
    buttonGroup->addButton(ui->tbnClass, 1);
    buttonGroup->addButton(ui->tbnMoney, 2);
    buttonGroup->addButton(ui->tbnHonor, 3);
    buttonGroup->addButton(ui->tbnSystem, 4);

    connect(buttonGroup, SIGNAL(buttonClicked(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));

    ui->tbnstdInfo->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::initializeWindowText()
{
    setWindowTitle(QString::fromUtf8("学生管理系统"));
    ui->tbnstdInfo->setText(QString::fromUtf8("学生信息"));
    ui->tbnClass->setText(QString::fromUtf8("课程安排"));
    ui->tbnMoney->setText(QString::fromUtf8("财务管理"));
    ui->tbnHonor->setText(QString::fromUtf8("荣誉展示"));
    ui->tbnSystem->setText(QString::fromUtf8("系统设置"));
}

void MainWindow::loadStudentData()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Skip loadStudentData because database is closed";
        return;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT id, student_id, name, gender, age, phone, address, class_name, avatar FROM student")) {
        qDebug() << "Query student failed:" << query.lastError().text();
        return;
    }

    QList<QMap<QString, QVariant>> studentData;
    while (query.next()) {
        QMap<QString, QVariant> row;
        row["id"] = query.value(0);
        row["student_id"] = query.value(1);
        row["name"] = query.value(2);
        row["gender"] = query.value(3);
        row["age"] = query.value(4);
        row["phone"] = query.value(5);
        row["address"] = query.value(6);
        row["class_name"] = query.value(7);
        row["avatar"] = query.value(8);
        studentData.append(row);
    }

    auto *studentWidget = findChild<studenInfoWight *>("pagesedinfo");
    if (!studentWidget) {
        qDebug() << "Cannot find student page named pagesedinfo";
        return;
    }

    studentWidget->setStudentData(studentData);
}
