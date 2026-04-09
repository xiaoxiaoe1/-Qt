#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    void checkAndCreateIntialUser();
    QString hashPassword(const QString& password);
    void on_loginButton_clickded();
    bool validataUser(const QString& username, const QString& password);
    void saveCredentials(const QString& username, const QString& password);
    QString encryptPassword(const QString& password);
    QString decryptPassword(const QString& encryptedPassword);
    bool loadCreadentials(QString& username, QString& password);
    QLineEdit* usernameLineEdit;
    QLineEdit* passwordLineEdit;
    QPushButton* loginButton;
    QPushButton* cancelButton;
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
