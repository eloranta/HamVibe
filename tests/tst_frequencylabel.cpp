#include <QtTest>
#include "frequencylabel.h"

class FrequencyLabelTest : public QObject
{
    Q_OBJECT

public:
    void initLabel(FrequencyLabel &label);

private slots:
    void leftClickDecrementsDigit();
    void rightClickIncrementsDigit();
    void clickOutsideDigitsDoesNothing();
};

static QPoint clickPointForChar(const FrequencyLabel &label, int charIndex)
{
    const QRect textRect = label.contentsRect();
    const QFontMetrics metrics(label.font());
    const QString text = label.text();
    const int textWidth = metrics.horizontalAdvance(text);

    int xStart = textRect.x();
    const Qt::Alignment align = label.alignment();
    if (align & Qt::AlignHCenter) {
        xStart = textRect.x() + (textRect.width() - textWidth) / 2;
    } else if (align & Qt::AlignRight) {
        xStart = textRect.x() + textRect.width() - textWidth;
    }

    int width = 0;
    for (int i = 0; i <= charIndex; ++i) {
        width += metrics.horizontalAdvance(text.at(i));
    }
    const int x = xStart + width - 1;
    const int y = textRect.center().y();
    return QPoint(x, y);
}

void FrequencyLabelTest::initLabel(FrequencyLabel &label)
{
    label.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label.setPrefix('A');
    label.setValue(10109000);
    label.resize(300, 40);
    label.show();
    QVERIFY(QTest::qWaitForWindowExposed(&label));
}

void FrequencyLabelTest::leftClickDecrementsDigit()
{
    FrequencyLabel label;
    initLabel(label);

    QSignalSpy spy(&label, &FrequencyLabel::valueChanged);

    const QPoint p = clickPointForChar(label, 2);
    QTest::mouseClick(&label, Qt::LeftButton, Qt::NoModifier, p);

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toInt(), 10099000);
}

void FrequencyLabelTest::rightClickIncrementsDigit()
{
    FrequencyLabel label;
    initLabel(label);

    QSignalSpy spy(&label, &FrequencyLabel::valueChanged);

    const QPoint p = clickPointForChar(label, 2);
    QTest::mouseClick(&label, Qt::RightButton, Qt::NoModifier, p);

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toInt(), 20109000);
}

void FrequencyLabelTest::clickOutsideDigitsDoesNothing()
{
    FrequencyLabel label;
    initLabel(label);

    QSignalSpy spy(&label, &FrequencyLabel::valueChanged);

    const QPoint p = clickPointForChar(label, 0);
    QTest::mouseClick(&label, Qt::LeftButton, Qt::NoModifier, p);

    QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(FrequencyLabelTest)
#include "tst_frequencylabel.moc"
