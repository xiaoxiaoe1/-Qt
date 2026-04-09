#ifndef HONORWIDGET_H
#define HONORWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>

class QPushButton;
class QScrollArea;
class QGridLayout;

constexpr int imgH=500;
constexpr int imgW=300;

// 直接定义可点击标签（不用新建文件）
class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = nullptr) : QLabel(parent) {
        setCursor(Qt::PointingHandCursor); // 鼠标变手型
    }
signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit clicked();
        }
        QLabel::mousePressEvent(event);
    }
};
namespace Ui {
class HonorWidget;
}

class HonorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HonorWidget(QWidget *parent = nullptr);
    ~HonorWidget();

private:
    void setupUI();
    void loadImageFromDatabase();
    void addImage();
    void addImageToWall(const QString& imagePath);
    void addImageToUI(const QPixmap& pixmap);
    void onImageClicked();
    void deleteImage();
    void reorderImages();
    void modifyImage();
    void deleteEmptyRecords(); // 删除空记录
    QPushButton* addButton;
    QPushButton* modifyButton;
    QPushButton* deleteButton;
    QScrollArea* scrollArea;
    QWidget* contentWidget;
    QGridLayout* gridLayout;
    ClickableLabel *selectedLabel;
    Ui::HonorWidget *ui;
};

#endif // HONORWIDGET_H
