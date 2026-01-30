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
    const bool isLeft = event->button() == Qt::LeftButton;
    const bool isRight = event->button() == Qt::RightButton;
    if (!isLeft && !isRight) {
        QLabel::mousePressEvent(event);
        return;
    }

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
    if (xRel < 0 || xRel >= textWidth) {
        QLabel::mousePressEvent(event);
        return;
    }

    const int charIndex = hitTestCharIndex(xRel);
    if (charIndex < 2 || charIndex > 9 || !stringValue.at(charIndex).isDigit()) {
        QLabel::mousePressEvent(event);
        return;
    }

    const int digitIndex = charIndex - 2;
    const int minValue = 30000;
    const int maxValue = 59990000;
    const int currentValue = value;
    QString digits = QString::number(currentValue).rightJustified(8, '0');

    int firstNonZero = 0;
    while (firstNonZero < digits.size() && digits.at(firstNonZero) == '0') {
        ++firstNonZero;
    }
    if (digitIndex < firstNonZero) {
        QLabel::mousePressEvent(event);
        return;
    }

    if (!adjustDigits(digits, digitIndex, isRight)) {
        QLabel::mousePressEvent(event);
        return;
    }

    const int newValue = digits.toInt();
    if (newValue >= minValue && newValue <= maxValue && newValue != currentValue) {
        setValue(newValue);
        emit valueChanged(value, prefix);
    }

    QLabel::mousePressEvent(event);
}

int FrequencyLabel::hitTestCharIndex(int x) const
{
    if (x < 0) {
        return -1;
    }

    const QFontMetrics metrics(font());
    int charIndex = -1;
    for (int i = 0; i < stringValue.size(); ++i) {
        const int width = metrics.horizontalAdvance(stringValue.left(i + 1));
        if (x < width) {
            charIndex = i;
            break;
        }
    }
    return charIndex;
}

bool FrequencyLabel::adjustDigits(QString &digits, int digitIndex, bool increment)
{
    int i = digitIndex;
    if (increment) {
        for (; i >= 0; --i) {
            if (digits.at(i) < '9') {
                digits[i] = QChar(digits.at(i).unicode() + 1);
                return true;
            }
            digits[i] = '0';
        }
    } else {
        for (; i >= 0; --i) {
            if (digits.at(i) > '0') {
                digits[i] = QChar(digits.at(i).unicode() - 1);
                return true;
            }
            digits[i] = '9';
        }
    }
    return false;
}

void FrequencyLabel::updateStringValue()
{
    const QChar prefixChar = prefix.isNull() ? QChar(' ') : prefix;
    stringValue = QString(prefixChar) + ' ' + QString::number(value).rightJustified(8, ' ');
    setText(stringValue);
}



