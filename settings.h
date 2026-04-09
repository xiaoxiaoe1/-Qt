#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QString>
class Settings
{
public:
    static Settings& instance();
    QSettings& getQSettings() {return settings;}
    QString getDataBasePath() const;
    void setDatabasePath(const QString& path);
    bool getCacheEnabled() const;
    void setCacheEnabled(bool enabled);
    QString GetLastUser() const;
    void setLastUser(const QString& user);

private:
    Settings();
    QSettings settings;
};


#endif // SETTINGS_H
