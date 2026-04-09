#include "honorwidget.h"
#include "ui_honorwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QLayoutItem>
#include <QSqlQuery>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include <QSqlError>
#include <QDate>
#include <QBuffer>


HonorWidget::HonorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HonorWidget),
    selectedLabel(nullptr) //定义selectedLabel
{
    ui->setupUi(this);
    setupUI();
    loadImageFromDatabase(); // 新增：初始化时加载数据库中的图片
}

HonorWidget::~HonorWidget()
{
    delete ui;

}

void HonorWidget::setupUI()
{

    //主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    //按钮布局
    QHBoxLayout* buttonLatout = new QHBoxLayout;
    //添加按钮
    addButton = new QPushButton("添加图片", this);
    connect(addButton, &QPushButton::clicked, this, &HonorWidget::addImage);
    buttonLatout->addWidget(addButton);
    //修改按钮
    modifyButton = new QPushButton("修改图片", this);
    connect(modifyButton, &QPushButton::clicked, this, &HonorWidget::modifyImage);
    buttonLatout->addWidget(modifyButton);
    //删除按钮
    deleteButton = new QPushButton("删除图片", this);
    connect(deleteButton, &QPushButton::clicked, this, &HonorWidget::deleteImage);
    buttonLatout->addWidget(deleteButton);
    mainLayout->addLayout(buttonLatout);
    //滚动区域
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); //允许内容区域调整大小
    //内容区域
    contentWidget = new QWidget(scrollArea);
    gridLayout = new QGridLayout(contentWidget); //网格布局
    contentWidget->setLayout(gridLayout);
    //设置滚动区域的内容
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
    //设置主布局
    setLayout(mainLayout);
}

//本地图片
void HonorWidget::loadImageFromDatabase()
{
    //清空布局中的所有内容
    QLayoutItem* item;
    while ((item = gridLayout->takeAt(0)) != nullptr) {
        // 先删除布局项对应的控件
        if (item->widget()) {
            item->widget()->deleteLater(); // 安全删除控件（QT事件循环处理）
        }
        delete item; //删除页面布局
    }

    // 检查数据库连接
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "数据库未连接";
        return;
    }

    //从数据库中加载图片
    QSqlQuery query("SELECT id, image_data FROM honor");
    
    // 检查查询是否成功
    if (!query.exec()) {
        qWarning() << "查询失败：" << query.lastError().text();
        return;
    }
    
    // 检查查询结果数量
    int count = 0;
    while (query.next()) {
        count++;
        int id = query.value(0).toInt();
        QByteArray imageData = query.value(1).toByteArray();
        
        // 打印调试信息
        qDebug() << "图片ID：" << id;
        qDebug() << "图片数据大小：" << imageData.size();

        //将二进制数据转换成QPixmap
        QPixmap pixmap;
        bool loaded = pixmap.loadFromData(imageData);
        qDebug() << "图片加载成功：" << loaded;

        if(!pixmap.isNull()) {
            //将图片显示在界面上
            ClickableLabel *imagLabel = new ClickableLabel(this);
            QPixmap scaledPixmap = pixmap.scaled(imgW, imgH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            imagLabel->setPixmap(scaledPixmap);
            imagLabel->setAlignment(Qt::AlignCenter);
            imagLabel->setStyleSheet("border: 1px solid #ccc; padding:5px");
            imagLabel->setProperty("id", id); // 设置ID属性

            connect(imagLabel, &ClickableLabel::clicked, this, &HonorWidget::onImageClicked);
            //动态添加到网格布局
            int row = gridLayout->count() / 3; //每行3张图片
            int col = gridLayout->count() % 3;
            gridLayout->addWidget(imagLabel, row, col);
        }
        else {
            qWarning() << "无法加载图片数据，ID：" << id;
        }
    }
    
    qDebug() << "总共查询到" << count << "条图片记录";

}

void HonorWidget::addImage()
{
    QString imagePath =QFileDialog::getOpenFileName(this, "选择图片", "", "图片文件(*.png *.jpg *.jepg *.bmp)");
    if(!imagePath.isEmpty()) addImageToWall(imagePath);
}

void HonorWidget::addImageToWall(const QString& imagePath)
{
    //加载图片
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载图片!");
        return;
    }
    //将图片转换成二进制
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG"); //保存为PNG格式
    // 插入数据库（已修复所有错误）
    QSqlQuery query;
    query.prepare("INSERT INTO honor (image_data, description, added_date) "
                  "VALUES (:image_data, :description, :added_date)");

    // ✅ 错误1：binValue → 改成 bindValue
    query.bindValue(":image_data", imageData);
    query.bindValue(":description", "未填写描述");
    query.bindValue(":added_date", QDate::currentDate());

    // ✅ 错误2：必须执行 query.exec()
    if (!query.exec()) {
        qDebug() << "插入失败：" << query.lastError().text();
        return;
    }
    //将图片显示在界面上
    //addImageToUI(pixmap);
    // 新增：插入成功后重新加载列表，确保新图片显示
    loadImageFromDatabase();

}

void HonorWidget::addImageToUI(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        qWarning() << "图片无效!";
        return;
    }
    QPixmap scaledPixmap = pixmap.scaled(imgW, imgH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ClickableLabel *imagLabel = new ClickableLabel(this);
    if (!imagLabel) {
        qWarning() << "无法创建 QLabel !";
        return;
    }
    imagLabel->setPixmap(scaledPixmap);
    imagLabel->setAlignment(Qt::AlignCenter);
    imagLabel->setStyleSheet("border: 1px solid #ccc; padding: 5px");

    connect(imagLabel, &ClickableLabel::clicked, this, &HonorWidget::onImageClicked);
    //动态添加到网格布局
    int row = gridLayout->count() / 3; //每行3张图片
    int col = gridLayout->count() % 3;
    gridLayout->addWidget(imagLabel, row, col);
}
//这里选中必须要定义selectedLabel，要不然就会结束进程
void HonorWidget::onImageClicked()
{
    if (selectedLabel) { //取消选中之前的图片样式
        selectedLabel->setStyleSheet("border: 1px solid #ccc; padding: 5px");
    }
    //"我知道你点的是哪张图片了，我把它记下来，后面删改就用它！"
    selectedLabel = qobject_cast<ClickableLabel*>(sender());
    if (selectedLabel) {
        selectedLabel->setStyleSheet("border: 2px solid red; padding: 5px");
    }
}

// 删除数据库中的空记录
void HonorWidget::deleteEmptyRecords()
{
    // 检查数据库连接
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "数据库未连接";
        return;
    }
    
    // 执行删除操作
    QSqlQuery query;
    query.prepare("DELETE FROM honor WHERE image_data IS NULL OR image_data = ''");
    
    if (!query.exec()) {
        qWarning() << "删除空记录失败：" << query.lastError().text();
        return;
    }
    
    // 打印删除的记录数
    int deletedCount = query.numRowsAffected();
    qDebug() << "删除了" << deletedCount << "条空记录";
    
    // 重新加载图片
    loadImageFromDatabase();
}

void HonorWidget::deleteImage()
{
    if (!selectedLabel) {
        QMessageBox::warning(this, "错误", "请选择一张照片!");
        return;
    }
    //确认删除
    if (QMessageBox::question(this, "确认删除", "确定要删除这站图片吗？") != QMessageBox::Yes) return;
    //获取当前选中图片对应的数据库id
    int id = selectedLabel->property("id").toInt();
    //从数据库删除记录
    QSqlQuery query;
    query.prepare("DELETE FROM honor WHERE id = :id");
    query.bindValue(":id", id);
    if(!query.exec()) {
        qWarning() << "错误数据失败:" << query.lastError().text();
        return;
    }

    //从界面删除
    gridLayout->removeWidget(selectedLabel);
    selectedLabel->deleteLater(); // 改用deleteLater更安全（QT事件循环处理）
    selectedLabel = nullptr;
    reorderImages(); //重新排列剩下的图片
}
//重新加载图片
void HonorWidget::reorderImages()
{
    //清空布局中的所有内容
    QLayoutItem* item;
    while ((item = gridLayout->takeAt(0)) != nullptr) { //item = gridLayout->takeAt(0)) != nullptr 把布局里所有图片一个一个拿出来
        if (item->widget()) item->widget()->setParent(nullptr); //移除 widget
        delete item; //删除布局项
    }
    loadImageFromDatabase(); //重新加载图片
}
//修改图片
void HonorWidget::modifyImage()
{
    if (!selectedLabel) {
        QMessageBox::warning(this, "错误", "请先选择一张图片!");
        return;
    }
    //打开文件对话框选择新图片
    QString imagePath = QFileDialog::getOpenFileName(this, "选择图片", "", "图片文件(*.png *.jpg *.jpeg *.bmp)");
    if (imagePath.isEmpty()) { // 修复：用isEmpty判断空路径（isNull是QVariant的判断）
        return;
    }
    //加载新图片
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载图片！");
        return;
    }

    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG"); // 保存为PNG格式
    //获取当前选择图片对应的数据库id
    int id = selectedLabel->property("id").toInt();
    //更新数据库
    QSqlQuery query;
    query.prepare("UPDATE honor SET image_data = :image_data WHERE id = :id");
    query.bindValue(":image_data", imageData);
    query.bindValue(":id", id);
    if(!query.exec()) {
        qWarning() << "更新数据失败:" <<query.lastError().text();
        return;
    }

    //更新界面上的图片
    QPixmap scaledPixmap = pixmap.scaled(imgW, imgH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    selectedLabel->setPixmap(scaledPixmap);
}

