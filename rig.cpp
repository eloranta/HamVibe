#include "Rig.h"

#include <QByteArray>

Rig::Rig(rig_model_t model, const QString &portName)
    : model(model)
    , portName(portName)
{
}

Rig::~Rig()
{
    close();
}

bool Rig::open()
{
    if (rig) {
        return true;
    }

    setError(QString());
    rig_set_debug_level(RIG_DEBUG_NONE);

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
    if (!rig) {
        return;
    }

    rig_close(rig);
    rig_cleanup(rig);
    rig = nullptr;
}

bool Rig::isOpen() const
{
    return rig != nullptr;
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
    const int freqStatus = rig_get_freq(rig, vfo, &freq);
    if (freqStatus != RIG_OK) {
        setError(QString("rig_get_freq failed: %1").arg(rigerror(freqStatus)));
        return false;
    }

    frequency = static_cast<int>(freq);

    return true;
}

bool Rig::setFrequency(qlonglong freq)
{
    return setFrequency(RIG_VFO_CURR, freq);
}

bool Rig::setFrequency(vfo_t vfo, qlonglong freq)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    const int setStatus = rig_set_freq(rig, vfo, static_cast<freq_t>(freq));
    if (setStatus != RIG_OK) {
        setError(QString("rig_set_freq failed: %1").arg(rigerror(setStatus)));
        return false;
    }

    return true;
}

bool Rig::setMode(rmode_t mode, pbwidth_t width)
{
    return setMode(RIG_VFO_CURR, mode, width);
}

bool Rig::setMode(vfo_t vfo, rmode_t mode, pbwidth_t width)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    const int setStatus = rig_set_mode(rig, vfo, mode, width);
    if (setStatus != RIG_OK) {
        setError(QString("rig_set_mode failed: %1").arg(rigerror(setStatus)));
        return false;
    }

    return true;
}

bool Rig::setSplit(bool enabled)
{
    return setSplit(RIG_VFO_A, RIG_VFO_B, enabled);
}

bool Rig::setSplit(vfo_t rxVfo, vfo_t txVfo, bool enabled)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    const split_t split = enabled ? RIG_SPLIT_ON : RIG_SPLIT_OFF;
    const int setStatus = rig_set_split_vfo(rig, rxVfo, split, txVfo);
    if (setStatus != RIG_OK) {
        setError(QString("rig_set_split_vfo failed: %1").arg(rigerror(setStatus)));
        return false;
    }

    return true;
}

bool Rig::getSplit(vfo_t rxVfo, bool *enabled, vfo_t *txVfo)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    split_t split = RIG_SPLIT_OFF;
    vfo_t tx = RIG_VFO_A;
    const int getStatus = rig_get_split_vfo(rig, rxVfo, &split, &tx);
    if (getStatus != RIG_OK) {
        setError(QString("rig_get_split_vfo failed: %1").arg(rigerror(getStatus)));
        return false;
    }

    if (enabled) {
        *enabled = (split == RIG_SPLIT_ON);
    }
    if (txVfo) {
        *txVfo = tx;
    }

    return true;
}

bool Rig::getActiveVfo(vfo_t *vfo)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    vfo_t current = RIG_VFO_CURR;
    const int getStatus = rig_get_vfo(rig, &current);
    if (getStatus != RIG_OK) {
        setError(QString("rig_get_vfo failed: %1").arg(rigerror(getStatus)));
        return false;
    }

    if (vfo) {
        *vfo = current;
    }

    return true;
}

bool Rig::setActiveVfo(vfo_t vfo)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    const int setStatus = rig_set_vfo(rig, vfo);
    if (setStatus != RIG_OK) {
        setError(QString("rig_set_vfo failed: %1").arg(rigerror(setStatus)));
        return false;
    }

    return true;
}

QString Rig::lastError() const
{
    return lastErrorMessage;
}

QString Rig::versionString()
{
    return QString::fromLatin1(rig_version());
}

void Rig::setError(const QString &message)
{
    lastErrorMessage = message;
}
