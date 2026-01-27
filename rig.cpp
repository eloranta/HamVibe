#include "rig.h"
#include <cmath>

namespace {
struct SmeterPoint {
    int raw;
    int value;
};

int scaleSmeter(int raw)
{
    static const SmeterPoint points[] = {
        {-60, 0},
        {-48, 1},
        {-36, 3},
        {-24, 5},
        {-16, 7},
        {0, 9},
        {20, 20},
        {40, 40},
        {60, 60},
    };

    if (raw <= points[0].raw) {
        return points[0].value;
    }
    const int last = static_cast<int>(sizeof(points) / sizeof(points[0])) - 1;
    if (raw >= points[last].raw) {
        return points[last].value;
    }

    for (int i = 1; i <= last; ++i) {
        if (raw <= points[i].raw) {
            const int x0 = points[i - 1].raw;
            const int y0 = points[i - 1].value;
            const int x1 = points[i].raw;
            const int y1 = points[i].value;
            const double t = static_cast<double>(raw - x0) / static_cast<double>(x1 - x0);
            return static_cast<int>(std::lround(y0 + t * (y1 - y0)));
        }
    }

    return points[last].value;
}
} // namespace

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
    if (!rig) {
        return;
    }

    rig_close(rig);
    rig_cleanup(rig);
    rig = nullptr;
}

QString Rig::lastError() const
{
    return lastErrorMessage;
}

void Rig::setError(const QString &message)
{
    lastErrorMessage = message;
}

bool Rig::readSMeter(int &value)
{
    return readSMeter(RIG_VFO_CURR, value);
}

bool Rig::readSMeter(vfo_t vfo, int &value)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    value_t level;
    const int status = rig_get_level(rig, vfo, RIG_LEVEL_STRENGTH, &level);
    if (status != RIG_OK) {
        setError(QString("rig_get_level(STRENGTH) failed: %1").arg(rigerror(status)));
        return false;
    }
    value = (level.i)/4 + 15;

    return true;
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

bool Rig::setMode(int mode, int width)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    const int status = rig_set_mode(
        rig,
        RIG_VFO_CURR,
        static_cast<rmode_t>(mode),
        static_cast<pbwidth_t>(width)
    );
    if (status != RIG_OK) {
        setError(QString("rig_set_mode failed: %1").arg(rigerror(status)));
        return false;
    }

    return true;
}

bool Rig::setPtt(bool enabled)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    const int status = rig_set_ptt(rig, RIG_VFO_CURR, enabled ? RIG_PTT_ON : RIG_PTT_OFF);
    if (status != RIG_OK) {
        setError(QString("rig_set_ptt failed: %1").arg(rigerror(status)));
        return false;
    }

    return true;
}

bool Rig::getPtt(bool &value)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    ptt_t ptt = RIG_PTT_OFF;
    const int status = rig_get_ptt(rig, RIG_VFO_CURR, &ptt);
    if (status != RIG_OK) {
        setError(QString("rig_get_ptt failed: %1").arg(rigerror(status)));
        return false;
    }

    value = (ptt == RIG_PTT_ON);
    return true;
}

bool Rig::readPower(double &watts)
{
    if (!rig) {
        setError("rig not open");
        return false;
    }

    value_t level;
    const int status = rig_get_level(rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER_METER_WATTS, &level);
    if (status != RIG_OK) {
        setError(QString("rig_get_level(RFPOWER) failed: %1").arg(rigerror(status)));
        return false;
    }

    watts = level.f;
    return true;
}
