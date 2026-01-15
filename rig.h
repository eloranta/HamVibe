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
    bool setSplit(bool enabled);
    bool setSplit(vfo_t rxVfo, vfo_t txVfo, bool enabled);
    bool getSplit(vfo_t rxVfo, bool *enabled, vfo_t *txVfo);
    bool getActiveVfo(vfo_t *vfo);
    bool setActiveVfo(vfo_t vfo);
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
