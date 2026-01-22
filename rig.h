#ifndef RIG_H
#define RIG_H

#include <QObject>
#include <hamlib/rig.h>

class Rig : public QObject
{
    Q_OBJECT
public:
    explicit Rig(rig_model_t model, const QString &portName, QObject *parent = nullptr);
    ~Rig();
    bool open();
    void close();
signals:
private:
    void setError(const QString &message);
    rig_model_t model;
    QString portName;
    RIG *rig = nullptr;
    QString lastErrorMessage;
};

#endif // RIG_H
