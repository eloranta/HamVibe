#include "Rig.h"

#include <QByteArray>
#include <QDebug>

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

bool Rig::getMode(rmode_t *mode, pbwidth_t *width)
{
    return getMode(RIG_VFO_CURR, mode, width);
}

bool Rig::getMode(vfo_t vfo, rmode_t *mode, pbwidth_t *width)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    rmode_t currentMode = RIG_MODE_NONE;
    pbwidth_t currentWidth = RIG_PASSBAND_NORMAL;
    const int getStatus = rig_get_mode(rig, vfo, &currentMode, &currentWidth);
    if (getStatus != RIG_OK) {
        setError(QString("rig_get_mode failed: %1").arg(rigerror(getStatus)));
        return false;
    }

    if (mode) {
        *mode = currentMode;
    }
    if (width) {
        *width = currentWidth;
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

    rmode_t currentMode = RIG_MODE_NONE;
    pbwidth_t currentWidth = RIG_PASSBAND_NORMAL;
    const int getStatus = rig_get_mode(rig, vfo, &currentMode, &currentWidth);
    if (getStatus == RIG_OK && currentMode == mode) {
        return true;
    }

    const int setStatus = rig_set_mode(rig, vfo, mode, width);
    if (setStatus != RIG_OK) {
        setError(QString("rig_set_mode failed: %1").arg(rigerror(setStatus)));
        return false;
    }

    emit modeChanged(mode);
    return true;
}

bool Rig::setPtt(bool enabled)
{
    return setPtt(RIG_VFO_CURR, enabled);
}

bool Rig::setPtt(vfo_t vfo, bool enabled)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    const ptt_t ptt = enabled ? RIG_PTT_ON : RIG_PTT_OFF;
    const int setStatus = rig_set_ptt(rig, vfo, ptt);
    if (setStatus != RIG_OK) {
        setError(QString("rig_set_ptt failed: %1").arg(rigerror(setStatus)));
        return false;
    }

    return true;
}

bool Rig::setMorseSpeed(int wpm)
{
    return setMorseSpeed(RIG_VFO_CURR, wpm);
}

bool Rig::setMorseSpeed(vfo_t vfo, int wpm)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    value_t level;
    level.i = wpm;
    const int setStatus = rig_set_level(rig, vfo, RIG_LEVEL_KEYSPD, level);
    if (setStatus != RIG_OK) {
        setError(QString("rig_set_level(KEYSPD) failed: %1").arg(rigerror(setStatus)));
        return false;
    }

    return true;
}

bool Rig::sendMorse(const QString &text)
{
    return sendMorse(RIG_VFO_CURR, text);
}

bool Rig::sendMorse(vfo_t vfo, const QString &text)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    const QByteArray bytes = text.toLocal8Bit();
    const int sendStatus = rig_send_morse(rig, vfo, bytes.constData());
    if (sendStatus != RIG_OK) {
        setError(QString("rig_send_morse failed: %1").arg(rigerror(sendStatus)));
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

bool Rig::getSWR(float *swr)
{
    return getSWR(RIG_VFO_CURR, swr);
}

bool Rig::getSWR(vfo_t vfo, float *swr)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    value_t level;
    const int getStatus = rig_get_level(rig, vfo, RIG_LEVEL_SWR, &level);
    if (getStatus != RIG_OK) {
        setError(QString("rig_get_level(SWR) failed: %1").arg(rigerror(getStatus)));
        return false;
    }

    if (swr) {
        *swr = level.f;
    }

    return true;
}

bool Rig::getStrength(int *strength)
{
    return getStrength(RIG_VFO_CURR, strength);
}

bool Rig::getStrength(vfo_t vfo, int *strength)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    value_t level;
    const int getStatus = rig_get_level(rig, vfo, RIG_LEVEL_STRENGTH, &level);
    if (getStatus != RIG_OK) {
        setError(QString("rig_get_level(STRENGTH) failed: %1").arg(rigerror(getStatus)));
        return false;
    }
    if (strength) {
        *strength = level.i;
    }

    return true;
}

bool Rig::getPower(float *power)
{
    return getPower(RIG_VFO_CURR, power);
}

bool Rig::getPower(vfo_t vfo, float *power)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    value_t level;
    const int getStatus = rig_get_level(rig, vfo, RIG_LEVEL_RFPOWER_METER_WATTS, &level);
    if (getStatus != RIG_OK) {
        setError(QString("rig_get_level(RFPOWER) failed: %1").arg(rigerror(getStatus)));
        return false;
    }
    qDebug() << level.f;
    if (power) {
        *power = level.f;
    }

    return true;
}

bool Rig::getALC(float *alc)
{
    return getALC(RIG_VFO_CURR, alc);
}

bool Rig::getALC(vfo_t vfo, float *alc)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    value_t level;
    const int getStatus = rig_get_level(rig, vfo, RIG_LEVEL_ALC, &level);
    if (getStatus != RIG_OK) {
        setError(QString("rig_get_level(ALC) failed: %1").arg(rigerror(getStatus)));
        return false;
    }

    if (alc) {
        *alc = level.f;
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
