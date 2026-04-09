// settings.h头文件引入，这里假设它包含Settings类的声明
#include "settings.h"

// 引入QSettings头文件，用于读写配置文件
#include <QSettings>
// 引入QString头文件，用于处理字符串
#include <QString>

// 获取Settings类的单例实例
Settings& Settings::instance()
{
    // 使用静态局部变量实现单例模式，保证全局只有一个Settings实例
    static Settings instance;
    return instance;
}

// Settings类的构造函数
// 初始化QSettings对象，使用config.ini作为配置文件名，采用Ini格式
Settings::Settings() : settings("config.ini", QSettings::IniFormat)
{

}

// 获取数据库路径
// 返回从配置文件中读取的数据库路径，如果没有找到则返回默认值"users.db"
QString Settings::getDataBasePath() const
{
    return settings.value("Database/Path", "E:/D/QT/deom/build-xuesheng-Desktop_Qt_5_12_9_MinGW_64_bit-Debug/student.db").toString();
}

// 设置数据库路径
// 将传入的路径值写入配置文件的"Database/Path"键下
void Settings::setDatabasePath(const QString &path)
{
    settings.setValue("Database/Path", path);
}

// 获取缓存是否启用的状态
// 返回从配置文件中读取的缓存启用状态，如果没有找到则返回默认值true
bool Settings::getCacheEnabled() const
{
    return settings.value("Login/CacheEnabled", true).toBool();
}

// 设置缓存启用状态
// 将传入的启用状态值写入配置文件的"Login/CacheEnabled"键下
void Settings::setCacheEnabled(bool enabled)
{
    settings.setValue("Login/CacheEnabled", enabled);
}

// 获取上次登录的用户
// 返回从配置文件中读取的上次登录用户，如果没有找到则返回空字符串
QString Settings::GetLastUser() const
{
    return settings.value("Login/LastUser", "").toString();
}

// 设置上次登录的用户
// 将传入的用户值写入配置文件的"Login/LastUser"键下
void Settings::setLastUser(const QString &user)
{
    settings.setValue("Login/LastUser", user);
}
