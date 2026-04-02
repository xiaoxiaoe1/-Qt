#ifndef STUDENINFOWIGHT_H
#define STUDENINFOWIGHT_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QByteArray>
#include <QTableWidgetItem>
#include <QTabWidget>


class QGroupBox;
class QTableWidgetItem;
namespace Ui {
class studenInfoWight;
}

class studenInfoWight : public QWidget
{
    Q_OBJECT

public:
    explicit studenInfoWight(QWidget *parent = nullptr);
    ~studenInfoWight();
    // 添加数据接收方法
    void setStudentData(const QList<QMap<QString, QVariant>> &data);


private slots:
    void on_pbAdd_clicked();
    void on_pbdeleteLine_clicked();
    void on_pbdeleteItem_clicked();
    void handItemChanged(QTableWidgetItem* item);

private:
    QGroupBox* createFormGroup();
    QGroupBox* createPhotoGroup();
    void handleDialogAccepted(QGroupBox* formGroup, QGroupBox* photoGroup);   
    void refreshTable();
    QByteArray photoData;
    Ui::studenInfoWight *ui;
};

#endif // STUDENINFOWIGHT_H
