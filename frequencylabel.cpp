#include "frequencylabel.h"

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
        emit valueChanged(value);
    }
    QLabel::mousePressEvent(event);
}

void FrequencyLabel::updateStringValue()
{
    const QChar prefixChar = prefix.isNull() ? QChar(' ') : prefix;
    stringValue = QString(prefixChar) + ' ' + QString::number(value).rightJustified(8, ' ');
    setText(stringValue);
}
