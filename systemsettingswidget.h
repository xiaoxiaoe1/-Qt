#ifndef SYSTEMSETTINGSWIDGET_H
#define SYSTEMSETTINGSWIDGET_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QCheckBox;
class QTextEdit;
class QGridLayout;
namespace Ui {
class SystemSettingsWidget;
}

class SystemSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemSettingsWidget(QWidget *parent = nullptr);
    ~SystemSettingsWidget();

private:
    void createUI();
    void browseDatabasePath();
    void loadSettings();
    void updatePassword();
    bool validatePasswordChage();
    void saveSettings();
    QLineEdit* dbPathEdit;
    QPushButton* browseBtn;
    QLineEdit* oldPwdEdit;
    QLineEdit* newPwdEdit;
    QLineEdit* confirmPwEdit;
    QCheckBox* cacheCheckBox;
    QPushButton* saveBtn;
    QTextEdit* versionInfoEdit;
    QLayout* mainLayout;
    Ui::SystemSettingsWidget *ui;
};

#endif // SYSTEMSETTINGSWIDGET_H
