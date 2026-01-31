#ifndef RIG_H
#define RIG_H

#include <QObject>
#include <hamlib/rig.h>

class Rig : public QObject
{
    Q_OBJECT
public:
    explicit Rig(uint32_t model, const QString &portName, QObject *parent = nullptr);
    ~Rig();
    bool open();
    void close();
    QString lastError() const;

    bool readSMeter(int &value);
    bool readSMeter(vfo_t vfo, int &value);

    bool readFrequency(int &frequency);
    bool readFrequency(vfo_t vfo, int &frequency);
    bool setFrequency(int frequency);
    bool setFrequency(vfo_t vfo, int frequency);
    bool setMode(int mode, int width = RIG_PASSBAND_NOCHANGE);
    bool setPtt(bool enabled);
    bool getPtt(bool &value);
    bool readPower(double &watts);
    bool readAlc(int &value);
    bool readAlc(vfo_t vfo, int &value);
    bool readSwr(int &value);
    bool readSwr(vfo_t vfo, int &value);
    bool readMode(rmode_t &mode);
    bool readMode(vfo_t vfo, rmode_t &mode);
    bool readVfo(vfo_t &vfo);
    bool readSplit(bool &enabled, vfo_t &txVfo);
    bool setCwSpeed(int wpm, vfo_t vfo = RIG_VFO_CURR);
    bool sendCw(const QString &text, vfo_t vfo = RIG_VFO_CURR);
signals:
private:
    void setError(const QString &message);
    rig_model_t model;
    QString portName;
    RIG *rig = nullptr;
    QString lastErrorMessage;
};

#endif // RIG_H
