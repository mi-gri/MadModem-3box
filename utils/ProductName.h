#pragma once
#include <QRegularExpression>
#include <QString>

// Product name of this fork. MultiModeModem (m³) is based on MadModem by
// Lucian-Ioan Papadopol, IZ6NNH (https://github.com/l-papadopol/MM).
//
// The program file, the application name (and with it the settings and data
// folders, see MadModemInstallation::adoptUpstreamUserData) and the proposed
// file names are MultiModeModem. Source identifiers, the CMake target and the
// WSJT-X UDP id keep the upstream name so upstream changes still merge. The
// translation dictionaries keep "MadModem"/"MM" as well; the name is replaced
// when a text is shown (brand()).
namespace ProductName {

inline QString longName() { return QStringLiteral("MultiModeModem"); }
inline QString shortName() { return QString::fromUtf8("m\xC2\xB3"); }   // m³
inline QString upstreamName() { return QStringLiteral("MadModem"); }

// Replaces the upstream product name in a visible text: "MadModem" becomes
// "MultiModeModem" everywhere, also in the folder and file names the texts
// propose ("Pictures/MadModem/SSTV"). The abbreviation "MM" becomes "m³",
// except as a word part or the maritime mobile suffix "/MM".
inline QString brand(QString text)
{
    if (!text.contains(QLatin1String("MadModem")) && !text.contains(QLatin1String("MM"))) {
        return text;
    }
    static const QRegularExpression shortRe(QStringLiteral("(?<![/\\\\\\w.])MM(?![/\\\\\\w])"));
    text.replace(QLatin1String("MadModem"), longName());
    text.replace(shortRe, shortName());
    return text;
}

} // namespace ProductName
