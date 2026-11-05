#pragma once
#include "MadModemVersion.h"
#include "MadModemBuildInfo.h"
#include "utils/BuildNumber.h"
#include "utils/ProductName.h"
#include <QString>

namespace MadModemIdentity {
inline QString windowTitle()
{
    // Fork name in the title bar; the version number stays MadModem's.
    return QStringLiteral("%1 (%2) " MADMODEM_VERSION_STRING " — build %3")
        .arg(ProductName::longName(), ProductName::shortName(), QString::fromLatin1(MadModemBuild::number()));
}
}
