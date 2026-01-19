#pragma once

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <hamlib/rig.h>

class Rig : public QObject
{
    Q_OBJECT
public:
    explicit Rig(rig_model_t model, const QString &portName, QObject *parent = nullptr);
    ~Rig();

    bool open();
    void close();
    bool isOpen() const;
    bool readFrequency(int &frequency);
    bool readFrequency(vfo_t vfo, int &frequency);
    bool setFrequency(qlonglong freq);
    bool setFrequency(vfo_t vfo, qlonglong freq);
    bool getMode(rmode_t *mode, pbwidth_t *width = nullptr);
    bool getMode(vfo_t vfo, rmode_t *mode, pbwidth_t *width = nullptr);
    bool setMode(rmode_t mode, pbwidth_t width = RIG_PASSBAND_NORMAL);
    bool setMode(vfo_t vfo, rmode_t mode, pbwidth_t width = RIG_PASSBAND_NORMAL);
    bool setPtt(bool enabled);
    bool setPtt(vfo_t vfo, bool enabled);
    bool setMorseSpeed(int wpm);
    bool setMorseSpeed(vfo_t vfo, int wpm);
    bool sendMorse(const QString &text);
    bool sendMorse(vfo_t vfo, const QString &text);
    bool setSplit(bool enabled);
    bool setSplit(vfo_t rxVfo, vfo_t txVfo, bool enabled);
    bool getSplit(vfo_t rxVfo, bool *enabled, vfo_t *txVfo);
    bool getActiveVfo(vfo_t *vfo);
    bool setActiveVfo(vfo_t vfo);
    bool getSWR(float *swr);
    bool getSWR(vfo_t vfo, float *swr);
    bool getStrength(int *strength);
    bool getStrength(vfo_t vfo, int *strength);
    bool getPower(float *power);
    bool getPower(vfo_t vfo, float *power);
    bool getALC(float *alc);
    bool getALC(vfo_t vfo, float *alc);
    bool setPower(float power);
    bool setPower(vfo_t vfo, float power);
    bool getAGC(int *agc);
    bool getAGC(vfo_t vfo, int *agc);
    bool getVoxEnabled(bool *enabled);
    bool getVoxEnabled(vfo_t vfo, bool *enabled);
    bool getAntenna(ant_t *ant);
    bool getAntenna(vfo_t vfo, ant_t *ant);
    bool setAntenna(ant_t ant);
    bool setAntenna(vfo_t vfo, ant_t ant);
    bool getTunerEnabled(bool *enabled);
    bool getTunerEnabled(vfo_t vfo, bool *enabled);
    bool setTunerEnabled(bool enabled);
    bool setTunerEnabled(vfo_t vfo, bool enabled);
    QString lastError() const;

    static QString versionString();

signals:
    void modeChanged(rmode_t mode);

private:
    void setError(const QString &message);

    rig_model_t model;
    QString portName;
    RIG *rig = nullptr;
    QString lastErrorMessage;
};
