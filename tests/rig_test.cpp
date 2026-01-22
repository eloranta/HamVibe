#include <QtTest/QtTest>

#include "rig.h"

class RigTest : public QObject
{
    Q_OBJECT
private slots:
    void setAndReadFrequency();
};

void RigTest::setAndReadFrequency()
{
    Rig rig(RIG_MODEL_TS590S, "COM7");

    if (!rig.open()) {
        qDebug() << "Hamlib rig_open failed:" << rig.lastError();
        return;
    }
    bool ok = rig.setFrequency(7000001);
    QVERIFY(ok);

    int frequency = 0;
    ok = rig.readFrequency(frequency);
    QVERIFY(ok);
    QCOMPARE(frequency, 7000001);
}

QTEST_MAIN(RigTest)

#include "rig_test.moc"
