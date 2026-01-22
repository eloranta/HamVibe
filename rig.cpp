#include "rig.h"

Rig::Rig(rig_model_t model, const QString &portName, QObject *parent)
    : QObject(parent)
    , model(model)
    , portName(portName)
{
}

Rig::~Rig()
{
    close();
}

bool Rig::open()
{
    if (rig) return true;

    setError("");
    rig_set_debug_level(RIG_DEBUG_NONE); // TODO:

    rig = rig_init(model);
    if (!rig) {
        setError("rig_init failed");
        return false;
    }
    const hamlib_token_t portToken = rig_token_lookup(rig, "rig_pathname");
    if (portToken != RIG_CONF_END) {
        const QByteArray portBytes = portName.toLocal8Bit();
        const int setStatus = rig_set_conf(rig, portToken, portBytes.constData());
        if (setStatus != RIG_OK) {
            setError(QString("rig_set_conf failed: %1").arg(rigerror(setStatus)));
            rig_cleanup(rig);
            rig = nullptr;
            return false;
        }
    }

    const int openStatus = rig_open(rig);
    if (openStatus != RIG_OK) {
        setError(QString("rig_open failed: %1").arg(rigerror(openStatus)));
        rig_cleanup(rig);
        rig = nullptr;
        return false;
    }

    return true;
}

void Rig::close()
{
}

QString Rig::lastError() const
{
    return lastErrorMessage;
}

void Rig::setError(const QString &message)
{
    lastErrorMessage = message;
}

bool Rig::readFrequency(int &frequency)
{
    return readFrequency(RIG_VFO_CURR, frequency);
}

bool Rig::readFrequency(vfo_t vfo, int &frequency)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    freq_t freq = 0;
    const int status = rig_get_freq(rig, vfo, &freq);
    if (status != RIG_OK) {
        setError(QString("rig_get_freq failed: %1").arg(rigerror(status)));
        return false;
    }

    frequency = static_cast<int>(freq);

    return true;
}

bool Rig::setFrequency(int freq)
{
    return setFrequency(RIG_VFO_CURR, freq);
}

bool Rig::setFrequency(vfo_t vfo, int freq)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    const int status = rig_set_freq(rig, vfo, static_cast<freq_t>(freq));
    if (status != RIG_OK) {
        setError(QString("rig_set_freq failed: %1").arg(rigerror(status)));
        return false;
    }

    return true;
}

