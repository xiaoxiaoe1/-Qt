#include "studeninfowight.h"
#include "ui_studeninfowight.h"

#include <QDialog>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QBuffer>
#include <QMessageBox>
#include "tabledelegates.h"
#include "studeninfowight.h"

studenInfoWight::studenInfoWight(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::studenInfoWight)
{
    ui->setupUi(this);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(100);

    //设置图标的男女性别
    ComboBoxDelegate* genderDelegate = new ComboBoxDelegate(this);
    genderDelegate->setItems(QStringList() << "男" << "女");
    ui->tableWidget->setItemDelegateForColumn(3, genderDelegate);
    //图片列代理
    ui->tableWidget->setItemDelegateForColumn(8, new ImageDelegate(this));
    refreshTable();
    //连接item修改信号
    connect(ui->tableWidget, &QTableWidget::itemChanged, this, &studenInfoWight::handItemChanged);
}

studenInfoWight::~studenInfoWight()
{
    delete ui;
}

void studenInfoWight::setStudentData(const QList<QMap<QString, QVariant> > &data)
{
    // 清空表格
    ui->tableWidget->blockSignals(true);   // ✅ 加
    ui->tableWidget->setRowCount(0);

    // 填充数据
    for (int i = 0; i < data.size(); i++) {
        const QMap<QString, QVariant> &rowData = data[i];
        ui->tableWidget->insertRow(i);

        // 处理普通字段
        for (int col = 0; col < ui->tableWidget->columnCount(); col++) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            // 处理头像
            if (col == ui->tableWidget->columnCount() - 1) {
                QByteArray photoData = rowData["avatar"].toByteArray();
                if (!photoData.isEmpty()) {
                    QPixmap photo;
                    photo.loadFromData(photoData);
                    // 调整图片大小并确保居中
                    QPixmap scaledPhoto = photo.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    item->setData(Qt::DecorationRole, photo.scaled(100, 100, Qt::KeepAspectRatio));
                    item->setData(Qt::UserRole, photoData);
                }
            } else {
                // 处理其他字段
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
                if (!key.isEmpty()) {
                    item->setText(rowData[key].toString());
                }
            }
            ui->tableWidget->setItem(i, col, item);

        }

        /****************************************************************************
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(rowData["id"].toString()));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(rowData["student_id"].toString()));
            ui->tableWidget->setItem(i, 2, new QTableWidgetItem(rowData["name"].toString()));
            ui->tableWidget->setItem(i, 3, new QTableWidgetItem(rowData["gender"].toString()));
            ui->tableWidget->setItem(i, 4, new QTableWidgetItem(rowData["age"].toString()));
            ui->tableWidget->setItem(i, 5, new QTableWidgetItem(rowData["phone"].toString()));
            ui->tableWidget->setItem(i, 6, new QTableWidgetItem(rowData["address"].toString()));
            ui->tableWidget->setItem(i, 7, new QTableWidgetItem(rowData["class_name"].toString()));
            ui->tableWidget->setItem(i, 8, new QTableWidgetItem(rowData["avatar"].toString()));

            **********************************************************************************/

    }
     ui->tableWidget->blockSignals(false);  // ✅ 加
}

//添加数据按钮
void studenInfoWight::on_pbAdd_clicked()
{
    //添加初始化窗口
    QDialog dialog(this);
    dialog.setWindowTitle("添加学生信息");
    dialog.setMinimumSize(600, 400);

    //初始化对话框布局
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    QHBoxLayout* contentLayout = new QHBoxLayout();
    mainLayout->addLayout(contentLayout);

    //添加表单和照片区域
    QGroupBox *fromGroup = createFormGroup();
    QGroupBox *photoGroup = createPhotoGroup();
    contentLayout->addWidget(fromGroup, 1);
    contentLayout->addWidget(photoGroup, 1);

    //配置按钮区域
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* btnConfirm = new QPushButton(tr("确认"));
    QPushButton* btnCancel = new QPushButton(tr("取消"));

    //配置按钮
    btnConfirm->setFixedWidth(150);
    btnCancel->setFixedWidth(150);

    //添加按钮到布局
    btnLayout->addStretch();
    btnLayout->addWidget(btnConfirm);
    btnLayout->addWidget(btnCancel);
    btnLayout->addStretch();

    //连接按钮信号
    connect(btnConfirm, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    mainLayout->addLayout(btnLayout);


    //显示对话框
    if(dialog.exec() == QDialog::Accepted) handleDialogAccepted(fromGroup, photoGroup);
}

QGroupBox *studenInfoWight::createFormGroup()
{
    //初始化表单信息
    QGroupBox* formGroup = new QGroupBox("基本信息");
    QFormLayout* formLayout = new QFormLayout(formGroup);

    //初始化控件
    QLineEdit* idEdit = new QLineEdit();
    idEdit->setObjectName("idEdit");
    QLineEdit* nameEdit = new QLineEdit();
    nameEdit->setObjectName("nameEdit");
    // 3. 性别（推荐用 QComboBox 下拉选择）
    QComboBox* genderCombo = new QComboBox();
    genderCombo->setObjectName("genderConbo");
    genderCombo->addItems({"男", "女"});
    // 4. 年龄（数字输入框 QSpinBox）
    QSpinBox* ageSpin = new QSpinBox();
    ageSpin->setObjectName("ageSoin");
    ageSpin->setRange(1, 100); // 限制年龄范围
    //5.电话
    QLineEdit* phoneEdit = new QLineEdit();
    phoneEdit->setObjectName("phoneEdit");
    //6.地址多行输入可用 (QTextEdit）
    QLineEdit* addressEdit = new QLineEdit();
    addressEdit->setObjectName("addressEdit");
    addressEdit->setMaximumHeight(60); //限制高度
    //7.班级名称
    QComboBox* classCombo = new QComboBox();
    classCombo->setObjectName("classCombo");
    classCombo->addItems({"计算机班", "人工智能班", "大数据班", "物联网班"});

    //配置控件
    formLayout->addRow("学号", idEdit);
    formLayout->addRow("姓名:", nameEdit);
    formLayout->addRow("性别:", genderCombo);
    formLayout->addRow("年龄:", ageSpin);
    formLayout->addRow("电话:", phoneEdit);
    formLayout->addRow("地址:", addressEdit);
    formLayout->addRow("班级名称:", classCombo);
    return formGroup;
}

//创建照片
QGroupBox *studenInfoWight::createPhotoGroup()
{
    QGroupBox* photoGroup = new QGroupBox("照片");
    QVBoxLayout* photoLayout = new QVBoxLayout();
    //初始化控件
    QLabel* lblPhotoPreview = new QLabel();
    QPushButton* btnSelectPhone = new QPushButton(tr("选择照片"));

    //配置控件
    lblPhotoPreview->setAlignment(Qt::AlignCenter);
    lblPhotoPreview->setMinimumSize(200, 200);
    btnSelectPhone->setFixedSize(200, 40);
    //添加控件到布局
    photoLayout->addWidget(lblPhotoPreview);
    photoLayout->addWidget(btnSelectPhone, 0, Qt::AlignCenter);
    //连接照片
    connect(btnSelectPhone, &QPushButton::clicked, [this, lblPhotoPreview]() {
        QString fileName = QFileDialog::getOpenFileName(
                    this,
                    tr("选择学生照片"),
                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                    tr("图片文件(*.png *.jpg *.jpeg)")
                    );
        //加载图片文件
        if(!fileName.isEmpty()) {
            QPixmap pixmap(fileName);
            if(!pixmap.isNull()) {
                pixmap = pixmap.scaled(//等比例缩放
                                       lblPhotoPreview->width() - 30,
                                       lblPhotoPreview->height() - 30,
                                       Qt::KeepAspectRatio
                                       );
                lblPhotoPreview->setPixmap(pixmap);
                QBuffer buffer(&photoData); //转换字节数组
                buffer.open(QIODevice::WriteOnly);
                pixmap.save(&buffer, "PNG");
            }
            else QMessageBox::warning(this, tr("错误"), tr("无法加载图片文件"));
        }
    });
    // 添加这行代码，将布局设置到photoGroup上能显示选择图片按钮
    photoGroup->setLayout(photoLayout);
    return photoGroup;
}

void studenInfoWight::handleDialogAccepted(QGroupBox *formGroup, QGroupBox *photoGroup)
{
    Q_UNUSED(photoGroup);

    //获取表单数据
    QLineEdit* idEdit = formGroup->findChild<QLineEdit*>("idEdit");
    QLineEdit* nameEdit = formGroup->findChild<QLineEdit*>("nameEdit");
    QComboBox* genderCombo = formGroup->findChild<QComboBox*>("genderConbo"); // 修复对象名称
    QSpinBox* ageSpin = formGroup->findChild<QSpinBox*>("ageSoin"); // 修复对象名称
    QLineEdit* phoneEdit = formGroup->findChild<QLineEdit*>("phoneEdit"); // 修复对象名称
    QLineEdit* addressEdit = formGroup->findChild<QLineEdit*>("addressEdit"); // 添加
    QComboBox* classCombo = formGroup->findChild<QComboBox*>("classCombo");

    // 检查所有控件是否找到
    if (!idEdit || !nameEdit || !genderCombo || !ageSpin || !phoneEdit || !addressEdit || !classCombo) {
        QMessageBox::warning(this, "错误", "无法获取表单数据！");
        return;
    }

    //校验数据
    if(idEdit->text().isEmpty() || nameEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("学号和姓名不能为空!"));
        return;
    }

    //检查学号唯一性
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM student WHERE id = ?");
    checkQuery.addBindValue(idEdit->text());
    if (checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("错误"), tr("学号 %1 已存在").arg(idEdit->text()));
        return;
    }
    //插入数据
    QSqlDatabase::database().transaction();//开始事务
    QSqlQuery query;
    query.prepare(
                "INSERT INTO student "
                "(student_id, name, gender, age, phone, address, class_name, avatar) "
                "VALUES (:student_id, :name, :gender, :age, :phone, :address, :class_name, :avatar)"
                );
    //绑定 8 个参数，与占位符数量一致
    query.bindValue(":student_id", idEdit->text());  // student_id
    query.bindValue(":name", nameEdit->text()); // name
    query.bindValue(":gender", genderCombo->currentText()); //gender
    query.bindValue(":age", ageSpin->value());   //age
    query.bindValue(":phone", phoneEdit->text()); //phone
    query.bindValue(":address", addressEdit->text()); //adresss
    query.bindValue(":class_name", classCombo->currentText()); //class_name
    query.bindValue(":avatar", photoData); // 绑定照片二进制数据

    // 执行SQL
    if (query.exec()) {
        //添加这行，提交
        QSqlDatabase::database().commit();
        QMessageBox::information(this, "成功", "学生信息添加成功！");
        refreshTable(); // 刷新表格显示
    } else {
        QMessageBox::warning(this, "错误", "添加失败：" + query.lastError().text());
    }
}
//更新项的内容，保存到项的数据和图片
void studenInfoWight::handItemChanged(QTableWidgetItem *item)
{
    ui->tableWidget->blockSignals(true);

    int row = item->row();
    int col = item->column();

    // ❗禁止修改编号
    if (col == 0) {
        QMessageBox::warning(this, "警告", "该列不可修改");
        ui->tableWidget->blockSignals(false);
        return;
    }

    // ❗防止空指针
    auto idItem = ui->tableWidget->item(row, 0);
    if (!idItem) {
        ui->tableWidget->blockSignals(false);
        return;
    }

    QString id = idItem->text();

    QString columnName = QStringList{
        "id", "student_id", "name", "gender", "age",
        "phone", "address", "class_name", "avatar"
    }[col];

    QSqlQuery query;
    query.prepare(QString("UPDATE student SET %1 = ? WHERE id = ?").arg(columnName));

    if (columnName == "avatar") {
        query.addBindValue(item->data(Qt::UserRole).toByteArray());
    } else {
        query.addBindValue(item->text().trimmed());
    }

    query.addBindValue(id);

    if (!query.exec()) {
        QMessageBox::warning(this, "错误", query.lastError().text());
    }

    ui->tableWidget->blockSignals(false);
}

void studenInfoWight::refreshTable()
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->setRowCount(0);

    QSqlQuery qurey("SELECT * FROM student");
    while (qurey.next()) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        for(int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);//设置文本居中
            //照片处理
            if(col == ui->tableWidget->columnCount() -1) {
                QByteArray photoData = qurey.value(col).toByteArray();
                if(!photoData.isEmpty()) {
                    QPixmap photo;
                    photo.loadFromData(photoData);
                    // 调整图片大小并确保居中
                    QPixmap scaledPhoto = photo.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    item->setData(Qt::DecorationRole, photo.scaled(100, 100, Qt::KeepAspectRatio));
                    item->setData(Qt::UserRole, photoData);
                }
            }
            else {
                item->setText(qurey.value(col).toString());
            }
            ui->tableWidget->setItem(row, col, item);
        }
    }
    ui->tableWidget->blockSignals(false);
}


void studenInfoWight::on_pbdeleteLine_clicked()
{
    auto selected = ui->tableWidget->selectionModel()->selectedRows();
    if(selected.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要删除的行！");
        return;
    }
    QSqlDatabase::database().transaction();
    foreach(const QModelIndex & index, selected) {
        QString id = ui->tableWidget->item(index.row(), 0)->text();
        QSqlQuery query;
        query.prepare("DELETE FROM student WHERE id = ?");
        query.addBindValue(id);
        if(!query.exec()) {
            QSqlDatabase::database().rollback();
            QMessageBox::critical(this, "错误", "删除失败:" + query.lastError().text());
            return;
        }
    }
    QSqlDatabase::database().commit();
    refreshTable();
}
//删除项
void studenInfoWight::on_pbdeleteItem_clicked()
{
    auto selecetd = ui->tableWidget->selectedItems();
    if(selecetd.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要删除的单元格");
        return;
    }
    QSqlDatabase::database().transaction();//开始事务
    foreach (QTableWidgetItem *item, selecetd) {
        int row = item->row();
        int col = item->column();
        // 跳过ID列，不允许删除ID
        if (col == 0) {
            continue;
        }
        // 获取ID值
        QString id = ui->tableWidget->item(row, 0)->text();

        // 根据列索引确定字段名
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
        default: continue; // 跳过无效列
        }
        {
            // 准备SQL语句
            QSqlQuery query;
            query.prepare("UPDATE student SET " + fieldName + " = ? WHERE id = ?");

            // 绑定两个参数
            query.addBindValue("");  // 设置为空字符串
            query.addBindValue(id);   // 设置ID

            // 执行SQL
            if(!query.exec()) {
                QSqlDatabase::database().rollback();
                QMessageBox::critical(this, "错误", "更新失败:" + query.lastError().text());
                return;
            }
        }
    }
    QSqlDatabase::database().commit();
    refreshTable();
}

