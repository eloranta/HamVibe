#ifndef DELEGATE_H
#define DELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <QSqlTableModel>
#include <QStringList>
#include <QVector>
#include <array>

class WwaDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void setModeVisibility(const std::array<bool, 4> &visible)
    {
        modeVisible = visible;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        if (option.state & QStyle::State_Selected) {
            painter->save();
            painter->setBrush(QColor("#dfefff"));
            painter->setPen(Qt::NoPen);
            painter->drawRect(option.rect);
            painter->restore();
        }
        int bits = index.data(Qt::DisplayRole).toInt();
        const QStringList symbols = {"CW", "PH", "FT8", "FT4"};

        QVector<int> visibleIndices;
        visibleIndices.reserve(4);
        for (int i = 0; i < 4; ++i) {
            if (modeVisible[i]) {
                visibleIndices.push_back(i);
            }
        }

        if (visibleIndices.isEmpty()) {
            return;
        }

        int boxWidth = option.rect.width() / visibleIndices.size();

        for (int i = 0; i < visibleIndices.size(); ++i) {
            const int modeIndex = visibleIndices[i];
            QRect boxRect(
                option.rect.left() + i * boxWidth,
                option.rect.top(),
                boxWidth,
                option.rect.height()
                );

            boxRect.adjust(2, 2, -2, -2);

            bool checked = bits & (1 << modeIndex);

            painter->save();
            painter->setPen(Qt::black);
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(boxRect, 4, 4);
            painter->setPen(checked ? Qt::red : Qt::gray);
            painter->drawText(boxRect, Qt::AlignCenter, symbols[modeIndex]);
            painter->restore();
        }
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        Q_UNUSED(parent);
        Q_UNUSED(option);
        Q_UNUSED(index);
        return nullptr;
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override
    {
        if (event->type() == QEvent::MouseButtonRelease) {
            auto *mouseEvent = static_cast<QMouseEvent*>(event);
            QVector<int> visibleIndices;
            visibleIndices.reserve(4);
            for (int i = 0; i < 4; ++i) {
                if (modeVisible[i]) {
                    visibleIndices.push_back(i);
                }
            }
            if (visibleIndices.isEmpty()) {
                return false;
            }

            int boxWidth = option.rect.width() / visibleIndices.size();
            if (boxWidth <= 0) {
                return false;
            }

            int clickedIndex = (mouseEvent->pos().x() - option.rect.x()) / boxWidth;
            if (clickedIndex < 0 || clickedIndex >= visibleIndices.size()) {
                return false;
            }

            const int modeIndex = visibleIndices[clickedIndex];

            int bits = index.data(Qt::DisplayRole).toInt();
            bits ^= (1 << modeIndex);

            model->setData(index, bits, Qt::EditRole);
            QSqlTableModel *sqlModel = qobject_cast<QSqlTableModel*>(model);
            if (sqlModel) {
                sqlModel->submitAll();
            }
            return true;
        }
        return false;
    }

private:
    std::array<bool, 4> modeVisible{{true, true, true, true}};
};

#endif // DELEGATE_H
