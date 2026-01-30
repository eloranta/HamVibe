#include <QtTest>
#include "frequencylabel.h"

class FrequencyLabelTest : public QObject
{
    Q_OBJECT

public:
    void clickTest(bool right, int freq, int index, int spyCount,int result = 0);
private slots:
    void rightClickIncrementsDigit();
    void leftClickIncrementsDigit();
};

QObject *createFrequencyLabelTest()
{
    return new FrequencyLabelTest();
}

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

void FrequencyLabelTest::clickTest(bool right, int freq, int index, int spyCount, int result)
{
    FrequencyLabel label;
    label.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label.setPrefix('A');
    label.setValue(freq);
    label.resize(300, 40);
    label.show();
    QVERIFY(QTest::qWaitForWindowExposed(&label));
    QSignalSpy spy(&label, &FrequencyLabel::valueChanged);

    const QPoint p = clickPointForChar(label, index);
    QTest::mouseClick(&label, right ? Qt::RightButton : Qt::LeftButton, Qt::NoModifier, p);

    QCOMPARE(spy.count(), spyCount);
    if (spy.count() > 0)
    {
        const QList<QVariant> args = spy.takeFirst();
        QCOMPARE(args.at(0).toInt(), result);
    }
}

void FrequencyLabelTest::rightClickIncrementsDigit()
{
    clickTest(true, 14000000, 9, 1, 14000001);
    clickTest(true, 14000000, 8, 1, 14000010);
    clickTest(true, 14000000, 7, 1, 14000100);
    clickTest(true, 14000000, 6, 1, 14001000);
    clickTest(true, 14000000, 5, 1, 14010000);
    clickTest(true, 14000000, 4, 1, 14100000);
    clickTest(true, 14000000, 3, 1, 15000000);
    clickTest(true, 14000000, 2, 1, 24000000);
    clickTest(true, 59990000, 9, 0);
    clickTest(true, 14000000, 1, 0);
    clickTest(true, 14000000, 0, 0);
}
void FrequencyLabelTest::leftClickIncrementsDigit()
{
    clickTest(false, 14000000, 9, 1, 13999999);
    clickTest(false, 14000000, 8, 1, 13999990);
    clickTest(false, 14000000, 7, 1, 13999900);
    clickTest(false, 14000000, 6, 1, 13999000);
    clickTest(false, 14000000, 5, 1, 13990000);
    clickTest(false, 14000000, 4, 1, 13900000);
    clickTest(false, 14000000, 3, 1, 13000000);
    clickTest(false, 14000000, 2, 1, 4000000);
    clickTest(false, 30000,    9, 0);
    clickTest(false, 14000000, 1, 0);
    clickTest(false, 14000000, 0, 0);
}

#include "frequencylabel_test.moc"
