#include <QtTest/QtTest>

#include "rig.h"

class RigTest : public QObject
{
    Q_OBJECT
private slots:
    void setAndReadFrequency();
private:
    static Rig rig;
};

void RigTest::setAndReadFrequency()
{
    if (!rig.open()) {
        qDebug() << "Hamlib rig_open failed:" << rig.lastError();
        return;
    }
    bool ok = rig.setFrequency(700010);
    QVERIFY(ok);

    int frequency = 0;
    ok = rig.readFrequency(frequency);
    QVERIFY(ok);
    QCOMPARE(frequency, 700010);
}

Rig RigTest::rig(RIG_MODEL_TS590S, "COM7");

QTEST_MAIN(RigTest)

#include "rig_test.moc"
