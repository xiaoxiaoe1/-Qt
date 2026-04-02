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
    
    // 检查student表是否存在
    QSqlQuery checkTableQuery("SELECT name FROM sqlite_master WHERE type='table' AND name='student'");
    bool studentTableExists = checkTableQuery.next();
    qDebug() << "student表存在：" << studentTableExists;
    
    if (studentTableExists) {
        // 检查表结构，看是否有student_id字段
        QSqlQuery checkColumnQuery("PRAGMA table_info(student)");
        bool hasStudentId = false;
        while (checkColumnQuery.next()) {
            QString columnName = checkColumnQuery.value(1).toString();
            if (columnName == "student_id") {
                hasStudentId = true;
                break;
            }
        }
        qDebug() << "student表有student_id字段：" << hasStudentId;
        
        // 检查student表是否有数据
        QSqlQuery countQuery("SELECT COUNT(*) FROM student");
        if (countQuery.next()) {
            int count = countQuery.value(0).toInt();
            qDebug() << "student表数据数量：" << count;
        }
    }
    
    // 尝试重命名旧表并创建新表
    if (!query.exec("ALTER TABLE student RENAME TO student_old;")) {
        qDebug() << "重命名表失败：" << query.lastError().text();
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS student ("  
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"  
               "student_id TEXT,"  
               "name TEXT,"  
               "gender TEXT,"  
               "age INTEGER,"  
               "phone TEXT,"  
               "address TEXT,"  
               "class_name TEXT,"  
               "avatar BLOB)"  // 这里改成 BLOB
               )) {
        qDebug() << "创建表失败：" << query.lastError().text();
    }

    if (!query.exec("INSERT INTO student SELECT * FROM student_old;")) {
        qDebug() << "插入数据失败：" << query.lastError().text();
    }
    
    // 检查student表是否有数据，如果没有，插入测试数据
    QSqlQuery countQuery("SELECT COUNT(*) FROM student");
    if (countQuery.next()) {
        int count = countQuery.value(0).toInt();
        qDebug() << "插入后student表数据数量：" << count;
        
        if (count == 0) {
            // 插入测试数据
            QString insertTestData = "INSERT INTO student (student_id, name, gender, age, phone, address, class_name) VALUES ('20410001', '张三', '男', 18, '13800138000', '北京市', '高一(1)班');";
            if (query.exec(insertTestData)) {
                qDebug() << "插入测试数据成功";
            } else {
                qDebug() << "插入测试数据失败：" << query.lastError().text();
            }
        }
    }
    
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

    //创建financialRecords表

    QString createFinancialTable =
            "CREATE TABLE IF NOT EXISTS financialRecords ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"    // 记录ID（主键）
            "student_id TEXT,"                         // 关联学生学号（和student表对应）
            "payment_date TEXT,"                       // 缴费日期（图片字段）
            "amount REAL,"                             // 金额（图片字段）
            "payment_type TEXT,"                       // 缴费类型（图片字段）
            "notes TEXT,"                              // 备注（对应图片里的tes/remark）
            "FOREIGN KEY (student_id) REFERENCES student(student_id)"  // 外键关联学生
            ");";

    if (!query.exec(createFinancialTable)) {
        qDebug() << "创建表失败：" << query.lastError().text();
        return;
    }
    qDebug() << "✅ createFinancialTable 表已存在或创建成功";

    // 插入financialRecords表的内容
//   QString insertFinanceSql =
//        "INSERT INTO financialRecords ("
//        "student_id, payment_date, amount, payment_type, notes"
//        ") VALUES ("
//       "'20410001', "                  // 学号（必须和 student 表里的 student_id 一致）
//        "'2026-04-02', "         // 缴费日期
//        "5600.00, "              // 金额
//        "'学费', "               // 缴费类型
//        "'2026年春季学期缴费'"   // 备注
//        ");";

//    if (!query.exec(insertFinanceSql)) {
//        qDebug() << "插入数据失败：" << query.lastError().text();
//        return;
//    }
//    qDebug() << "✅ 财务数据插入成功";



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

