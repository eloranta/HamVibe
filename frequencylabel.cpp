#include "frequencylabel.h"

#include <QFontMetrics>
#include <QMouseEvent>

FrequencyLabel::FrequencyLabel(QWidget *parent)
    : QLabel(parent)
{
}

void FrequencyLabel::setValue(int value)
{
    if (value < 30000) {
        value = 30000;
    } else if (value > 59990000) {
        value = 59990000;
    }
    this->value = value;
    updateStringValue();
}

void FrequencyLabel::setPrefix(const QChar &prefix)
{
    this->prefix = prefix;
    updateStringValue();
}

void FrequencyLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
        const QRect textRect = contentsRect();
        const QFontMetrics metrics(font());
        const int textWidth = metrics.horizontalAdvance(stringValue);
        int xStart = textRect.x();
        const Qt::Alignment align = alignment();
        if (align & Qt::AlignHCenter) {
            xStart = textRect.x() + (textRect.width() - textWidth) / 2;
        } else if (align & Qt::AlignRight) {
            xStart = textRect.x() + textRect.width() - textWidth;
        }

        const int xRel = event->pos().x() - xStart;
        if (xRel >= 0 && xRel < textWidth) {
            int charIndex = -1;
            int width = 0;
            for (int i = 0; i < stringValue.size(); ++i) {
                width += metrics.horizontalAdvance(stringValue.at(i));
                if (xRel < width) {
                    charIndex = i;
                    break;
                }
            }
            if (charIndex >= 2 && charIndex <= 9) {
                const int digitIndex = charIndex - 2;
                emit valueChanged(digitIndex, prefix);
            }
        }
    }
    QLabel::mousePressEvent(event);
}

void FrequencyLabel::updateStringValue()
{
    const QChar prefixChar = prefix.isNull() ? QChar(' ') : prefix;
    stringValue = QString(prefixChar) + ' ' + QString::number(value).rightJustified(8, ' ');
    setText(stringValue);
}
