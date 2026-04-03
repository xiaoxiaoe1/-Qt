#ifndef STUDENINFOWIGHT_H
#define STUDENINFOWIGHT_H

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QWidget>

class QGroupBox;
class QTableWidgetItem;

namespace Ui {
class studenInfoWight;
}

// 学生信息页负责学生档案的展示、增删以及表格内联修改。
class studenInfoWight : public QWidget
{
    Q_OBJECT

public:
    explicit studenInfoWight(QWidget *parent = nullptr);
    ~studenInfoWight() override;

    void setStudentData(const QList<QMap<QString, QVariant>> &data);

private slots:
    void on_pbAdd_clicked();
    void on_pbdeleteLine_clicked();
    void on_pbdeleteItem_clicked();
    void handItemChanged(QTableWidgetItem *item);

private:
    QGroupBox *createFormGroup();
    QGroupBox *createPhotoGroup();
    void handleDialogAccepted(QGroupBox *formGroup, QGroupBox *photoGroup);
    void refreshTable();

    QByteArray photoData;
    Ui::studenInfoWight *ui;
};

#endif // STUDENINFOWIGHT_H
