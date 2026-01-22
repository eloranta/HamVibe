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
    rig_set_debug_level(RIG_DEBUG_NONE);

    return true;
}

void Rig::close()
{
}

void Rig::setError(const QString &message)
{
    lastErrorMessage = message;
}

