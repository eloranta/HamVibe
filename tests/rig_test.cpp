#include <QtTest/QtTest>

#include "rig.h"

class RigTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void readSMeter();
    void setPtt();

    void setAndReadFrequency();
private:
    static Rig rig;
};

void RigTest::initTestCase()
{
    if (!rig.open()) {
        qDebug() << "Hamlib rig_open failed:" << rig.lastError();
        QFAIL("Hamlib rig_open failed");
    }
}

void RigTest::cleanupTestCase()
{
    rig.close();
}

void RigTest::setAndReadFrequency()
{
    bool ok = rig.setFrequency(700010);
    QVERIFY(ok);

    int frequency = 0;
    ok = rig.readFrequency(frequency);
    QVERIFY(ok);
    QCOMPARE(frequency, 700010);
}

void RigTest::readSMeter()
{
    int value = 0;
    const bool ok = rig.readSMeter(value);
    QVERIFY(ok);
}

void RigTest::setPtt()
{
    bool ok = rig.setPtt(true);
    QVERIFY(ok);
    ok = rig.setPtt(false);
    QVERIFY(ok);
}


Rig RigTest::rig(RIG_MODEL_TS590S, "COM7");

QTEST_MAIN(RigTest)

#include "rig_test.moc"
