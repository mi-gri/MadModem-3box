#pragma once
#include "MadModemVersion.h"
#include "MadModemBuildInfo.h"
#include <QString>

namespace MadModemIdentity {
inline QString windowTitle()
{
    // 3box fork: own name in the title bar; the version and build stay upstream's.
    return QStringLiteral("MadModem-3box " MADMODEM_VERSION_STRING " — build " MADMODEM_BUILD_NUMBER);
}
}
