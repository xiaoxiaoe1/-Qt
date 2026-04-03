#include "studeninfowight.h"
#include "ui_studeninfowight.h"

#include "tabledelegates.h"

#include <QBuffer>
#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVBoxLayout>

studenInfoWight::studenInfoWight(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::studenInfoWight)
{
    ui->setupUi(this);

    ui->pbAdd->setText(QString::fromUtf8("新增学生"));
    ui->pbdeleteItem->setText(QString::fromUtf8("清空字段"));
    ui->pbdeleteLine->setText(QString::fromUtf8("删除学生"));

    const QStringList headers = {
        QString::fromUtf8("编号"),
        QString::fromUtf8("学号"),
        QString::fromUtf8("姓名"),
        QString::fromUtf8("性别"),
        QString::fromUtf8("年龄"),
        QString::fromUtf8("电话"),
        QString::fromUtf8("地址"),
        QString::fromUtf8("班级"),
        QString::fromUtf8("头像")
    };
    for (int column = 0; column < headers.count(); ++column) {
        if (ui->tableWidget->horizontalHeaderItem(column)) {
            ui->tableWidget->horizontalHeaderItem(column)->setText(headers[column]);
        }
    }

    ui->tableWidget->verticalHeader()->setDefaultSectionSize(100);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);

    auto *genderDelegate = new ComboBoxDelegate(this);
    genderDelegate->setItems({QString::fromUtf8("男"), QString::fromUtf8("女")});
    ui->tableWidget->setItemDelegateForColumn(3, genderDelegate);
    ui->tableWidget->setItemDelegateForColumn(8, new ImageDelegate(this));

    refreshTable();
    connect(ui->tableWidget, &QTableWidget::itemChanged, this, &studenInfoWight::handItemChanged);
}

studenInfoWight::~studenInfoWight()
{
    delete ui;
}

void studenInfoWight::setStudentData(const QList<QMap<QString, QVariant>> &data)
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->setRowCount(0);

    for (int rowIndex = 0; rowIndex < data.size(); ++rowIndex) {
        const auto &rowData = data[rowIndex];
        ui->tableWidget->insertRow(rowIndex);

        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            auto *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);

            if (col == 8) {
                const QByteArray avatarData = rowData.value("avatar").toByteArray();
                if (!avatarData.isEmpty()) {
                    QPixmap avatar;
                    avatar.loadFromData(avatarData);
                    item->setData(Qt::DecorationRole,
                                  avatar.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    item->setData(Qt::UserRole, avatarData);
                }
            } else {
                QString key;
                switch (col) {
                case 0: key = "id"; break;
                case 1: key = "student_id"; break;
                case 2: key = "name"; break;
                case 3: key = "gender"; break;
                case 4: key = "age"; break;
                case 5: key = "phone"; break;
                case 6: key = "address"; break;
                case 7: key = "class_name"; break;
                default: break;
                }

                item->setText(rowData.value(key).toString());
            }

            ui->tableWidget->setItem(rowIndex, col, item);
        }
    }

    ui->tableWidget->blockSignals(false);
}

void studenInfoWight::on_pbAdd_clicked()
{
    photoData.clear();

    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("新增学生"));
    dialog.setMinimumSize(760, 440);

    auto *mainLayout = new QVBoxLayout(&dialog);
    auto *contentLayout = new QHBoxLayout();
    mainLayout->addLayout(contentLayout);

    QGroupBox *formGroup = createFormGroup();
    QGroupBox *photoGroup = createPhotoGroup();
    contentLayout->addWidget(formGroup, 3);
    contentLayout->addWidget(photoGroup, 2);

    auto *buttonLayout = new QHBoxLayout();
    auto *confirmButton = new QPushButton(QString::fromUtf8("确认"), &dialog);
    auto *cancelButton = new QPushButton(QString::fromUtf8("取消"), &dialog);

    buttonLayout->addStretch();
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    connect(confirmButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        handleDialogAccepted(formGroup, photoGroup);
    }
}

QGroupBox *studenInfoWight::createFormGroup()
{
    auto *formGroup = new QGroupBox(QString::fromUtf8("基本信息"));
    auto *formLayout = new QFormLayout(formGroup);

    auto *idEdit = new QLineEdit();
    idEdit->setObjectName("idEdit");

    auto *nameEdit = new QLineEdit();
    nameEdit->setObjectName("nameEdit");

    auto *genderCombo = new QComboBox();
    genderCombo->setObjectName("genderCombo");
    genderCombo->addItems({QString::fromUtf8("男"), QString::fromUtf8("女")});

    auto *ageSpin = new QSpinBox();
    ageSpin->setObjectName("ageSpin");
    ageSpin->setRange(1, 100);

    auto *phoneEdit = new QLineEdit();
    phoneEdit->setObjectName("phoneEdit");

    auto *addressEdit = new QLineEdit();
    addressEdit->setObjectName("addressEdit");

    auto *classCombo = new QComboBox();
    classCombo->setObjectName("classCombo");
    classCombo->addItems({
        QString::fromUtf8("计算机班"),
        QString::fromUtf8("人工智能班"),
        QString::fromUtf8("大数据班"),
        QString::fromUtf8("物联网班")
    });

    formLayout->addRow(QString::fromUtf8("学号："), idEdit);
    formLayout->addRow(QString::fromUtf8("姓名："), nameEdit);
    formLayout->addRow(QString::fromUtf8("性别："), genderCombo);
    formLayout->addRow(QString::fromUtf8("年龄："), ageSpin);
    formLayout->addRow(QString::fromUtf8("电话："), phoneEdit);
    formLayout->addRow(QString::fromUtf8("地址："), addressEdit);
    formLayout->addRow(QString::fromUtf8("班级："), classCombo);

    return formGroup;
}

QGroupBox *studenInfoWight::createPhotoGroup()
{
    auto *photoGroup = new QGroupBox(QString::fromUtf8("学生头像"));
    auto *photoLayout = new QVBoxLayout(photoGroup);

    auto *photoPreview = new QLabel();
    photoPreview->setAlignment(Qt::AlignCenter);
    photoPreview->setMinimumSize(220, 220);
    photoPreview->setStyleSheet("border: 1px dashed #6f7f93; border-radius: 12px;");

    auto *selectButton = new QPushButton(QString::fromUtf8("选择图片"));
    selectButton->setFixedWidth(160);

    photoLayout->addWidget(photoPreview, 1);
    photoLayout->addWidget(selectButton, 0, Qt::AlignCenter);

    connect(selectButton, &QPushButton::clicked, [this, photoPreview]() {
        const QString fileName = QFileDialog::getOpenFileName(
                    this,
                    QString::fromUtf8("选择头像"),
                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                    QString::fromUtf8("图片文件 (*.png *.jpg *.jpeg *.bmp)"));

        if (fileName.isEmpty()) {
            return;
        }

        QPixmap pixmap(fileName);
        if (pixmap.isNull()) {
            QMessageBox::warning(this, QString::fromUtf8("加载失败"), QString::fromUtf8("无法加载所选图片。"));
            return;
        }

        photoPreview->setPixmap(pixmap.scaled(photoPreview->size() - QSize(20, 20),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));

        photoData.clear();
        QBuffer buffer(&photoData);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
    });

    return photoGroup;
}

void studenInfoWight::handleDialogAccepted(QGroupBox *formGroup, QGroupBox *photoGroup)
{
    Q_UNUSED(photoGroup);

    auto *idEdit = formGroup->findChild<QLineEdit *>("idEdit");
    auto *nameEdit = formGroup->findChild<QLineEdit *>("nameEdit");
    auto *genderCombo = formGroup->findChild<QComboBox *>("genderCombo");
    auto *ageSpin = formGroup->findChild<QSpinBox *>("ageSpin");
    auto *phoneEdit = formGroup->findChild<QLineEdit *>("phoneEdit");
    auto *addressEdit = formGroup->findChild<QLineEdit *>("addressEdit");
    auto *classCombo = formGroup->findChild<QComboBox *>("classCombo");

    if (!idEdit || !nameEdit || !genderCombo || !ageSpin || !phoneEdit || !addressEdit || !classCombo) {
        QMessageBox::warning(this, QString::fromUtf8("控件错误"), QString::fromUtf8("表单控件初始化不完整。"));
        return;
    }

    if (idEdit->text().trimmed().isEmpty() || nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("数据不完整"), QString::fromUtf8("学号和姓名不能为空。"));
        return;
    }

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM student WHERE student_id = ?");
    checkQuery.addBindValue(idEdit->text().trimmed());
    if (checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, QString::fromUtf8("学号重复"), QString::fromUtf8("该学号已经存在。"));
        return;
    }

    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare(
                "INSERT INTO student (student_id, name, gender, age, phone, address, class_name, avatar) "
                "VALUES (:student_id, :name, :gender, :age, :phone, :address, :class_name, :avatar)");
    query.bindValue(":student_id", idEdit->text().trimmed());
    query.bindValue(":name", nameEdit->text().trimmed());
    query.bindValue(":gender", genderCombo->currentText());
    query.bindValue(":age", ageSpin->value());
    query.bindValue(":phone", phoneEdit->text().trimmed());
    query.bindValue(":address", addressEdit->text().trimmed());
    query.bindValue(":class_name", classCombo->currentText());
    query.bindValue(":avatar", photoData);

    if (!query.exec()) {
        QSqlDatabase::database().rollback();
        QMessageBox::warning(this, QString::fromUtf8("新增失败"), query.lastError().text());
        return;
    }

    QSqlDatabase::database().commit();
    QMessageBox::information(this, QString::fromUtf8("操作成功"), QString::fromUtf8("学生信息已添加。"));
    refreshTable();
}

void studenInfoWight::handItemChanged(QTableWidgetItem *item)
{
    if (!item) {
        return;
    }

    ui->tableWidget->blockSignals(true);

    const int row = item->row();
    const int col = item->column();

    if (col == 0) {
        QMessageBox::warning(this, QString::fromUtf8("禁止修改"), QString::fromUtf8("编号列不允许直接修改。"));
        refreshTable();
        ui->tableWidget->blockSignals(false);
        return;
    }

    auto *idItem = ui->tableWidget->item(row, 0);
    if (!idItem) {
        ui->tableWidget->blockSignals(false);
        return;
    }

    const QStringList columnNames = {
        "id", "student_id", "name", "gender", "age",
        "phone", "address", "class_name", "avatar"
    };

    const QString columnName = columnNames.value(col);
    QSqlQuery query;
    query.prepare(QString("UPDATE student SET %1 = ? WHERE id = ?").arg(columnName));

    if (columnName == "avatar") {
        query.addBindValue(item->data(Qt::UserRole).toByteArray());
    } else {
        query.addBindValue(item->text().trimmed());
    }

    query.addBindValue(idItem->text());

    if (!query.exec()) {
        QMessageBox::warning(this, QString::fromUtf8("更新失败"), query.lastError().text());
    }

    ui->tableWidget->blockSignals(false);
}

void studenInfoWight::refreshTable()
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->setRowCount(0);

    QSqlQuery query("SELECT id, student_id, name, gender, age, phone, address, class_name, avatar FROM student");
    while (query.next()) {
        const int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            auto *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);

            if (col == 8) {
                const QByteArray avatarData = query.value(col).toByteArray();
                if (!avatarData.isEmpty()) {
                    QPixmap avatar;
                    avatar.loadFromData(avatarData);
                    item->setData(Qt::DecorationRole,
                                  avatar.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    item->setData(Qt::UserRole, avatarData);
                }
            } else {
                item->setText(query.value(col).toString());
            }

            ui->tableWidget->setItem(row, col, item);
        }
    }

    ui->tableWidget->blockSignals(false);
}

void studenInfoWight::on_pbdeleteLine_clicked()
{
    const auto selectedRows = ui->tableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请至少选择一行数据。"));
        return;
    }

    QSqlDatabase::database().transaction();

    for (const QModelIndex &index : selectedRows) {
        const QString id = ui->tableWidget->item(index.row(), 0)->text();
        QSqlQuery query;
        query.prepare("DELETE FROM student WHERE id = ?");
        query.addBindValue(id);

        if (!query.exec()) {
            QSqlDatabase::database().rollback();
            QMessageBox::critical(this, QString::fromUtf8("删除失败"), query.lastError().text());
            return;
        }
    }

    QSqlDatabase::database().commit();
    refreshTable();
}

void studenInfoWight::on_pbdeleteItem_clicked()
{
    const auto selectedItems = ui->tableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请至少选择一个单元格。"));
        return;
    }

    QSqlDatabase::database().transaction();

    for (QTableWidgetItem *item : selectedItems) {
        const int row = item->row();
        const int col = item->column();

        if (col == 0) {
            continue;
        }

        const QString id = ui->tableWidget->item(row, 0)->text();

        QString fieldName;
        switch (col) {
        case 1: fieldName = "student_id"; break;
        case 2: fieldName = "name"; break;
        case 3: fieldName = "gender"; break;
        case 4: fieldName = "age"; break;
        case 5: fieldName = "phone"; break;
        case 6: fieldName = "address"; break;
        case 7: fieldName = "class_name"; break;
        case 8: fieldName = "avatar"; break;
        default: break;
        }

        if (fieldName.isEmpty()) {
            continue;
        }

        QSqlQuery query;
        query.prepare(QString("UPDATE student SET %1 = ? WHERE id = ?").arg(fieldName));

        if (fieldName == "avatar") {
            query.addBindValue(QByteArray());
        } else {
            query.addBindValue(QString());
        }

        query.addBindValue(id);

        if (!query.exec()) {
            QSqlDatabase::database().rollback();
            QMessageBox::critical(this, QString::fromUtf8("清空失败"), query.lastError().text());
            return;
        }
    }

    QSqlDatabase::database().commit();
    refreshTable();
}
