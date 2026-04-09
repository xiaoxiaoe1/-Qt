#include "systemsettingswidget.h"
#include "ui_systemsettingswidget.h"

#include <QLineEdit>  // 单行输入框（输用户名、密码、学号用）
#include <QPushButton>
#include <QCheckBox>  // 复选框（打√的选项，如：记住密码、我已同意）
#include <QGridLayout>
#include <QTextEdit>  // 多行文本框（输大段文字、显示日志、详情
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>

#include "settings.h"
SystemSettingsWidget::SystemSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystemSettingsWidget)
{
    ui->setupUi(this);
    createUI();
    loadSettings();
}

SystemSettingsWidget::~SystemSettingsWidget()
{
    delete ui;
}
//界面设置
void SystemSettingsWidget::createUI()
{
    dbPathEdit = new QLineEdit(this);
    browseBtn = new QPushButton("浏览...", this);
    oldPwdEdit = new QLineEdit(this);
    newPwdEdit = new QLineEdit(this);
    confirmPwEdit = new QLineEdit(this);
    cacheCheckBox = new QCheckBox("记住登录信息", this);
    saveBtn = new QPushButton("保存", this);
    versionInfoEdit = new QTextEdit(this);

    oldPwdEdit->setEchoMode(QLineEdit::Password);
    newPwdEdit->setEchoMode(QLineEdit::Password);
    confirmPwEdit->setEchoMode(QLineEdit::Password);

    versionInfoEdit->setPlainText("轩轩学生管理系统 1.0\n 开发环境：QT C++11 Win11");
    versionInfoEdit->setReadOnly(true);

    // 左侧设置区域
    QWidget *settingsWidget = new QWidget(this);
    QGridLayout *settingsLayout = new QGridLayout(settingsWidget);
    settingsLayout->addWidget(new QLabel("数据库路径:", this), 0, 0);
    settingsLayout->addWidget(dbPathEdit, 0, 1);
    settingsLayout->addWidget(browseBtn, 0, 2);
    settingsLayout->addWidget(new QLabel("旧密码:", this), 1, 0);
    settingsLayout->addWidget(oldPwdEdit, 1, 1, 1, 2);
    settingsLayout->addWidget(new QLabel("新密码:", this), 2, 0);
    settingsLayout->addWidget(newPwdEdit, 2, 1, 1, 2);
    settingsLayout->addWidget(new QLabel("确认密码:", this), 3, 0);
    settingsLayout->addWidget(confirmPwEdit, 3, 1, 1, 3);
    settingsLayout->addWidget(cacheCheckBox, 4, 0, 1, 3);
    settingsLayout->addWidget(saveBtn, 5, 1, 1, 2);
    settingsLayout->addWidget(versionInfoEdit, 6, 0, 1, 3);
    
    // 右侧图片区域
    QWidget *rightImageWidget = new QWidget(this);
    rightImageWidget->setMinimumSize(400, 800); // 设置最小大小
    QVBoxLayout *rightImageLayout = new QVBoxLayout(rightImageWidget);
    rightImageLayout->setContentsMargins(0, 0, 0, 0); // 移除边距
    
    // 添加右侧图片标签
    QLabel *rightImageLabel = new QLabel(rightImageWidget);
    rightImageLabel->setAlignment(Qt::AlignCenter);
    rightImageLabel->setStyleSheet("border: 1px solid #ccc; padding: 0px;"); // 移除内边距
    rightImageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 设置大小策略为扩展
    
    // 尝试加载本地图片
    QPixmap rightPixmap;
    if (rightPixmap.load(":/C:/Users/27957/Pictures/QT/90.jpg")) {
        // 直接使用固定大小，确保图片足够大
        rightImageLabel->setPixmap(rightPixmap.scaled(400, 800, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        // 如果图片不存在，显示默认文本
        rightImageLabel->setText("学生管理系统\n\n右侧图片区域");
        rightImageLabel->setStyleSheet("border: 1px solid #ccc; padding: 10px; font-size: 16px; color: #666;");
    }
    
    rightImageLayout->addWidget(rightImageLabel);

    // 主布局
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->addWidget(settingsWidget, 1);  // 设置区域占1份
    hLayout->addWidget(rightImageWidget, 1);   // 右侧图片区域占1份
    hLayout->setContentsMargins(0, 0, 0, 0); // 移除边距
    hLayout->setSpacing(0); // 移除间距
    mainLayout = hLayout;
    setLayout(mainLayout);

    connect(browseBtn, &QPushButton::clicked, this, &SystemSettingsWidget::browseDatabasePath);
    connect(saveBtn, &QPushButton::clicked, this, &SystemSettingsWidget::saveSettings);

}
//选择数据库位置
void SystemSettingsWidget::browseDatabasePath()
{
    QString path = QFileDialog::getSaveFileName(
                this,
                "选择数据库文件",
                "",
                "SQLite Databases (*.db *.sqlite)"
                );
    if (!path.isEmpty()) dbPathEdit->setText(path);
}
//加载当前设置
void SystemSettingsWidget::loadSettings()
{
    dbPathEdit->setText(Settings::instance().getDataBasePath());
    cacheCheckBox->setChecked(Settings::instance().getCacheEnabled());
}
//修改密码
void SystemSettingsWidget::updatePassword()
{
    if (!validatePasswordChage()) return;
    QString newHash = QString (QCryptographicHash::hash(
                                   newPwdEdit->text().toUtf8(),
                                   QCryptographicHash::Sha256
                                   ).toHex());
    QSqlQuery query;
    query.prepare("UPDATE users SET password = ? WHERE username = ?");
    query.addBindValue(newHash);
    query.addBindValue(Settings::instance().GetLastUser());
    if (!query.exec()) {
        QMessageBox::critical(this, "错误" , "密码更新失败:" + query.lastError().text());
        return;
    }
    QMessageBox::information(this, "提示", "密码更新成功");
}

//验证密码的有效性
bool SystemSettingsWidget::validatePasswordChage()
{
    if (newPwdEdit->text() != confirmPwEdit->text()) {
        QMessageBox::warning(this, "错误", "新密码与确认密码不一样");
        return false;
    }

    QString currentUser = Settings::instance().GetLastUser();
    if (currentUser.isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到当前用户");
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = ?");
    query.addBindValue(currentUser);
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "错误", "数据库查询失败:" + query.lastError().text());
        return false;
    }

    QString storedHash = query.value(0).toString();
    QString inputHash = QString (QCryptographicHash::hash(
                                     oldPwdEdit->text().toUtf8(),
                                     QCryptographicHash::Sha256
                                     ).toHex());
    if (storedHash != inputHash) {
        QMessageBox::warning(this, "错误", "旧密码不正确");
        return false;
    }
    return true;
}
//保存系统设置
void SystemSettingsWidget::saveSettings()
{
    QString newDbpath = dbPathEdit->text();
    Settings::instance().setDatabasePath(newDbpath);
    Settings::instance().setCacheEnabled(cacheCheckBox->isChecked());

    if (!newPwdEdit->text().isEmpty()) {
        updatePassword();
    }

    if (newDbpath != Settings::instance().getDataBasePath()) {
        QMessageBox::information(this, "提示" , "数据库路径修改将在重启后生效");
    }
}
