#ifndef TABLEDELEGATES_H
#define TABLEDELEGATES_H

#include <QStyledItemDelegate>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QBuffer>
#include <QEvent>
#include <QPainter>
#include <QFileDialog>
#include <QFile>
#include <QMouseEvent>
#include "studeninfowight.h"
class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ComboBoxDelegate(QObject* parent = nullptr): QStyledItemDelegate(parent) {}
    //设置下拉菜单的选项
    void setItems(const QStringList& items)
    {
        m_items = items; //设置下来菜单选项
    }
    //创建编辑器
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);

        QComboBox* editor = new QComboBox(parent);
        editor->addItems(m_items);//添加选项到下拉菜单
        return editor;
    }
    //设置数据模型
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {
        QComboBox* comboBox = static_cast<QComboBox*>(editor);
        QString value = comboBox->currentText();
        model->setData(index, value, Qt::EditRole);
    }
private:
    QStringList m_items;
};

// 图片委托类，用于处理表格中的图片显示和编辑
class ImageDelegate : public QStyledItemDelegate {
public:
    // 构造函数
    explicit ImageDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    
    // 创建编辑器
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        Q_UNUSED(option); // 未使用的参数
        Q_UNUSED(index);  // 未使用的参数

        // 创建一个标签作为编辑器
        QLabel* editor = new QLabel(parent);
        return editor;
    }
    
    // 设置模型数据
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        // 将编辑器转换为标签
        QLabel* label = qobject_cast<QLabel*>(editor);
        if (label) {
            QByteArray imageData; // 用于存储图片的二进制数据
            // 首先获取指针
            const QPixmap* pixmapPtr = label->pixmap();
            // 检查指针是否有效
            if (pixmapPtr) {
                // 通过解引用获取对象
                QPixmap pixmap = *pixmapPtr;
                // 检查图片是否有效
                if (!pixmap.isNull()) {
                    // 创建缓冲区，用于将图片转换为二进制数据
                    QBuffer buffer(&imageData);
                    buffer.open(QIODevice::WriteOnly);
                    // 将图片保存为 PNG 格式的二进制数据
                    pixmap.save(&buffer, "PNG");
                }
            }
            // 将二进制数据保存到模型中
            model->setData(index, imageData, Qt::UserRole);
        }
    }
    
    // 绘制函数，用于自定义项的绘制
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        // 从模型中获取二进制图片数据
        QByteArray imageData = index.data(Qt::UserRole).toByteArray();
        
        // 如果没有图片数据，使用默认绘制
        if (imageData.isEmpty()) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }
        
        // 将二进制数据转换为 QPixmap
        QPixmap pixmap;
        pixmap.loadFromData(imageData);
        
        // 如果图片加载失败，使用默认绘制
        if (pixmap.isNull()) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }
        
        // 绘制图片
        QRect rect = option.rect; // 获取单元格的矩形区域
        const int photoSize = 100; // 图片大小
        
        // 缩放图片，保持宽高比
        QPixmap scaledPixmap = pixmap.scaled(photoSize, photoSize, Qt::KeepAspectRatio);
        
        // 绘制缩放后的图片
        painter->drawPixmap(rect, scaledPixmap);
    }
    
    // 处理与编辑器相关的事件
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override {
        // 检查是否是双击事件
        if (event->type() == QEvent::MouseButtonDblClick) {
            // 将事件转换为鼠标事件
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            
            // 检查是否是左键双击
            if (mouseEvent->button() == Qt::LeftButton) {
                // 弹出文件对话框选择新图片
                QString imagePath = QFileDialog::getOpenFileName(
                            nullptr,"选择图片","", "图片文件 (*.png *.jpg *.bmp)"
                            );
                
                // 如果用户选择了有效的图片路径
                if (!imagePath.isEmpty()) {
                    // 将图片加载为二进制数据
                    QFile file(imagePath);
                    if (file.open(QIODevice::ReadOnly)) {
                        QByteArray imageData = file.readAll();
                        file.close();
                        // 将二进制数据保存到模型中
                        model->setData(index, imageData, Qt::UserRole);
                    }
                }
                // 如果用户取消选择，保留原来的图片
                return true; // 事件已处理
            }
        }
        
        // 其他事件交给父类处理
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
};
#endif // TABLEDELEGATES_H
