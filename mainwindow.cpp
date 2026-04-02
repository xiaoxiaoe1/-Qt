#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QButtonGroup>
#include <QSqlError>    //数据库报错
#include <QSqlDatabase> //连接数据库
#include <QSqlQuery>    //操作数据库
#include <QMessageBox>
#include <QDebug>
#include "schedulewidget.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // ======================
    // 第一步:数据库连接（自动连接）
    // ======================
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("student.db");//创建的数据库名字


    if(!db.open()) {
        QMessageBox::warning(this, "错误", "数据库连接失败！");
        return;
    }

    if(!db.open()) {
        // 打印详细错误到控制台（Qt Creator的「应用程序输出」窗口）
        qDebug() << "数据库打开失败，详细错误：" << db.lastError().text();
        // 弹窗也显示错误，方便直接看
        QMessageBox::warning(this, "错误",
                             "数据库连接失败！\n错误信息：" + db.lastError().text());
        return;
    }
    // 连接成功的日志
    qDebug() << "✅ 数据库连接成功！驱动：" << db.driverName()
             << "，数据库文件：" << db.databaseName();

    // 创建学生表（如果不存在）
    /****************
    QSqlQuery createQuery;
    createQuery.exec("CREATE TABLE IF NOT EXISTS student ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "student_id TEXT,"
                     "name TEXT,"
                     "gender TEXT,"
                     "age INTEGER,"
                     "phone TEXT,"
                     "address TEXT,"
                     "class_name TEXT,"
                     "avatar TEXT)");
     ***********************/

    QSqlQuery query;
    query.exec("ALTER TABLE student RENAME TO student_old;");

    query.exec("CREATE TABLE IF NOT EXISTS student ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "student_id TEXT,"
               "name TEXT,"
               "gender TEXT,"
               "age INTEGER,"
               "phone TEXT,"
               "address TEXT,"
               "class_name TEXT,"
               "avatar BLOB)");  // 这里改成 BLOB

    query.exec("INSERT INTO student SELECT * FROM student_old;");
    query.exec("DROP TABLE student_old;");

    //创建schedule 表（如果不存在）

    QString  createSql = R"(
                         CREATE TABLE IF NOT EXISTS schedule (
                         id INTEGER PRIMARY KEY AUTOINCREMENT,
                         date TEXT NOT NULL,
                         time TEXT NOT NULL,
                         course_name TEXT NOT NULL,
                         UNIQUE(date, time) ON CONFLICT REPLACE -- 关键：添加联合唯一约束
                         )
                         )";

    if (!query.exec(createSql)) {
        qDebug() << "创建表失败：" << query.lastError().text();
        return;
    }
    qDebug() << "✅ schedule 表已存在或创建成功";


    // ======================
    // 【第二步：再初始化UI！此时数据库已就绪】
    // ======================
    ui->setupUi(this);

    // ======================
    // 【第三步：UI初始化、按钮组、加载数据】
    // ======================
    loadStudentData();
    //按钮组
    QButtonGroup *bgp = new QButtonGroup(this);
    bgp->addButton(ui->tbnstdInfo,0);
    bgp->addButton(ui->tbnClass,1);
    bgp->addButton(ui->tbnMoney,2);
    bgp->addButton(ui->tbnHonor,3);
    bgp->addButton(ui->tbnSystem,4);

    // ✅【这行是关键！】点击按钮发送ID，切换页面
    connect(bgp, SIGNAL(buttonClicked(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
    //默认第一页
    bgp->button(0)->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadStudentData()
{
    // 检查数据库连接状态
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "数据库未连接";
        return;
    }
    qDebug() << "数据库已连接";
    // 检查是否存在student表
    QSqlQuery checkTableQuery("SELECT name FROM sqlite_master WHERE type='table' AND name='student'");
    if (checkTableQuery.next()) {
        qDebug() << "student表存在";
    } else {
        qDebug() << "student表不存在";
    }
    // 执行查询语句
    QSqlQuery query;
    if (!query.exec("select * FROM student")) {
        qDebug() << "查询失败：" << query.lastError().text();
        return;
    }
    qDebug() << "查询执行成功";

    // 收集数据
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

    // 将数据传递给表格
    studenInfoWight *studentWidget = findChild<studenInfoWight*>("pagesedinfo");
    if (studentWidget) {
        studentWidget->setStudentData(studentData);
    } else {
        qDebug() << "未找到 studenInfoWight 实例";
    }
}

