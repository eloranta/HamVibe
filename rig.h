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
    bool readFrequency(int &frequency);
    bool readFrequency(vfo_t vfo, int &frequency);
    bool setFrequency(int frequency);
    bool setFrequency(vfo_t vfo, int frequency);
signals:
private:
    void setError(const QString &message);
    rig_model_t model;
    QString portName;
    RIG *rig = nullptr;
    QString lastErrorMessage;
};

#endif // RIG_H
