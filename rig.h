#pragma once

#include <QString>
#include <QtGlobal>
#include <hamlib/rig.h>

class Rig
{
public:
    Rig(rig_model_t model, const QString &portName);
    ~Rig();

    bool open();
    void close();
    bool isOpen() const;
    bool readFrequency(qlonglong *freqOut);
    bool readFrequency(vfo_t vfo, qlonglong *freqOut);
    bool setFrequency(qlonglong freq);
    bool setFrequency(vfo_t vfo, qlonglong freq);
    bool setSplit(bool enabled);
    bool setSplit(vfo_t rxVfo, vfo_t txVfo, bool enabled);
    bool getSplit(vfo_t rxVfo, bool *enabled, vfo_t *txVfo);
    bool getActiveVfo(vfo_t *vfo);
    bool setActiveVfo(vfo_t vfo);
    QString lastError() const;

    static QString versionString();

private:
    void setError(const QString &message);

    rig_model_t model;
    QString portName;
    RIG *rig = nullptr;
    QString lastErrorMessage;
};
