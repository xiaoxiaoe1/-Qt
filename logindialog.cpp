#include "logindialog.h"
#include "ui_logindialog.h"

#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QDialog>
#include <QFile>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include "settings.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    // 移除帮助按钮（?）
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    checkAndCreateIntialUser();
    setWindowTitle(QString::fromUtf8("轩轩学生课程管理"));
    setWindowIcon(QIcon(":/icon/xuesheng.png"));
    setFixedSize(400, 300);
    //创建控件
    QLabel* usernameLabel = new QLabel("用户名:", this);
    QLabel* passwordLabel = new QLabel("密 码:", this);
    usernameLineEdit = new QLineEdit(this);
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password); //密码输入模式
    loginButton = new QPushButton("登录", this);
    cancelButton = new QPushButton("取消", this);

    //布局
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(usernameLabel, 0, 0);
    mainLayout->addWidget(usernameLineEdit, 0, 1);
    mainLayout->addWidget(passwordLabel, 1, 0);
    mainLayout->addWidget(passwordLineEdit, 1, 1);
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout, 2, 0, 1, 2); //跨两列
    setLayout(mainLayout);
    //连接信号和槽
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::on_loginButton_clickded);
    connect(cancelButton, &QPushButton::clicked, this, &LoginDialog::reject);
    //尝试加载缓存的登陆信息
    QString cacheUsername, cachePassword;
    if (Settings::instance().getCacheEnabled()) {
        if (loadCreadentials(cacheUsername, cachePassword)) {
            usernameLineEdit->setText(cacheUsername);
            passwordLineEdit->setText(cachePassword);
        }
    }
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::checkAndCreateIntialUser()
{
    const QString initalUsername = "admin"; //初始化用户名和密码
    const QString initalPassword = "admin123";

    // 检查数据库连接
    QSqlDatabase db = QSqlDatabase::contains() ? QSqlDatabase::database() : QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("student.db");
    
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "数据库打开失败：" << db.lastError().text();
            return;
        }
    }

    QSqlQuery query(db);
    query.exec("SELECT COUNT(*) FROM users");
    if (query.next() && query.value(0).toInt() == 0) { //检查users表是否为空，表为空，插入初始用户
        QString hashedInitialPassword = hashPassword(initalPassword);
        query.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
        query.bindValue(":username", initalUsername);
        query.bindValue(":password", hashedInitialPassword);
        if (!query.exec()) qDebug() << "插入用户失败" << query.lastError().text();
    }
}
//加密算法
QString LoginDialog::hashPassword(const QString &password)
{
    QByteArray passwoedBytes = password.toUtf8();
    QByteArray hashBytes = QCryptographicHash::hash(passwoedBytes, QCryptographicHash::Sha256);
    return QString(hashBytes.toHex());
}
//登陆验证功能
void LoginDialog::on_loginButton_clickded()
{
    QString username = usernameLineEdit->text();
    QString password = passwordLineEdit->text();

    if (validataUser (username, password)) {
        //登陆成功,保存登陆信息
        saveCredentials(username, password);
        //将当前登陆的用户名保存到Settings
        Settings::instance().setLastUser(username);
        accept(); //关闭对话框
    }
    else QMessageBox::warning(this, "登录失败", "用户名或密码失败");
}
//验证登陆
bool LoginDialog::validataUser(const QString &username, const QString &password)
{
    QString hashedPassword = hashPassword(password);
    
    // 检查数据库连接
    QSqlDatabase db = QSqlDatabase::contains() ? QSqlDatabase::database() : QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("student.db");
    
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "数据库打开失败：" << db.lastError().text();
            return false;
        }
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", hashedPassword); // 使用哈希后的密码
    if (!query.exec()) {
        qDebug() << "查询错误:" <<query.lastError().text();
        return false;
    }
    return query.next();
}
//保存功能
void LoginDialog::saveCredentials(const QString &username, const QString &password)
{
    // 清理配置文件中的冗余项
    Settings::instance().getQSettings().remove("General");
    
    // 只保存必要的信息（通过setLastUser已经保存了LastUser）
    // 不再保存密码到配置文件，提高安全性
}
//加密和解密密钥
const QByteArray encryptionKey = "your_encryption_Key";
//加密函数
QString LoginDialog::encryptPassword(const QString &password)
{
    QByteArray passwordByte = password.toUtf8();
    QByteArray encryptedBytes;
    for (int i = 0; i < passwordByte.size(); ++i) {
        encryptedBytes.append(passwordByte[i]  ^ encryptionKey[i % encryptionKey.size()] );
    }
    return encryptedBytes.toBase64();
}

//解密函数
QString LoginDialog::decryptPassword(const QString &encryptedPassword)
{
    QByteArray encryptedBytes = QByteArray::fromBase64(encryptedPassword.toUtf8());
    QByteArray decryptedBytes;
    for (int i = 0; i < encryptedBytes.size(); ++i) {
        decryptedBytes.append(encryptedBytes[i]  ^ encryptionKey[i % encryptionKey.size()] );
    }
    return QString(decryptedBytes);
}

//解密的函数
bool LoginDialog::loadCreadentials(QString &username, QString &password)
{
    username = Settings::instance().getQSettings().value("username").toString();
    QString encryptedPassword = Settings::instance().getQSettings().value("password").toString();
    //如果用户名和加密后的密码不为空，则解密密码
    if (!username.isEmpty() && !encryptedPassword.isEmpty()) {
        password = decryptPassword(encryptedPassword);
        return true;
    }
    return false;
}



