#ifndef TABLEDELEGATES_H
#define TABLEDELEGATES_H

#include <QBuffer>
#include <QComboBox>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyledItemDelegate>

// Delegate for fixed option columns such as gender.
class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ComboBoxDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void setItems(const QStringList &items)
    {
        m_items = items;
    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);

        auto *editor = new QComboBox(parent);
        editor->addItems(m_items);
        return editor;
    }

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override
    {
        auto *comboBox = static_cast<QComboBox *>(editor);
        model->setData(index, comboBox->currentText(), Qt::EditRole);
    }

private:
    QStringList m_items;
};

// Delegate for avatar preview and double click replacement.
class ImageDelegate : public QStyledItemDelegate
{
public:
    explicit ImageDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        return new QLabel(parent);
    }

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override
    {
        auto *label = qobject_cast<QLabel *>(editor);
        if (!label || !label->pixmap()) {
            return;
        }

        QByteArray imageData;
        QBuffer buffer(&imageData);
        buffer.open(QIODevice::WriteOnly);
        label->pixmap()->save(&buffer, "PNG");
        model->setData(index, imageData, Qt::UserRole);
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        const QByteArray imageData = index.data(Qt::UserRole).toByteArray();
        if (imageData.isEmpty()) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        QPixmap pixmap;
        pixmap.loadFromData(imageData);
        if (pixmap.isNull()) {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        const QPixmap scaledPixmap = pixmap.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const QPoint drawPoint(option.rect.center().x() - scaledPixmap.width() / 2,
                               option.rect.center().y() - scaledPixmap.height() / 2);
        painter->drawPixmap(drawPoint, scaledPixmap);
    }

    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override
    {
        Q_UNUSED(option);

        if (event->type() != QEvent::MouseButtonDblClick) {
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        }

        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton) {
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        }

        const QString imagePath = QFileDialog::getOpenFileName(
                    nullptr,
                    "Choose Avatar",
                    QString(),
                    "Image Files (*.png *.jpg *.jpeg *.bmp)");

        if (imagePath.isEmpty()) {
            return true;
        }

        QFile file(imagePath);
        if (file.open(QIODevice::ReadOnly)) {
            model->setData(index, file.readAll(), Qt::UserRole);
        }

        return true;
    }
};

#endif // TABLEDELEGATES_H
