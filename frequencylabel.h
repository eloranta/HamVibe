#ifndef FREQUENCYLABEL_H
#define FREQUENCYLABEL_H

#include <QLabel>
#include <QString>

class QMouseEvent;

class FrequencyLabel : public QLabel
{
    Q_OBJECT

public:
    explicit FrequencyLabel(QWidget *parent = nullptr);

    void setValue(int value);
    void setPrefix(const QChar &prefix);

signals:
    void valueChanged(int value, QChar prefix);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void updateStringValue();
    int value = 0;
    QChar prefix;
    QString stringValue;
};

#endif // FREQUENCYLABEL_H
