// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// The TX audio level: the digital gain applied to every generated TX sample,
// which sets the drive and so the RF power of a rig keyed by audio.
//
// The level is kept as a whole number, 0 .. 450: tenths of a dB below full
// scale, counted upwards - 450 is 0.0 dB, 0 is -45.0 dB. The operator sees and
// enters it in dB with one decimal. Ported from the JTDX 3box Tx level control
// (txlevel.h), so both programs store the same numbers.
//
// Plain C++ without Qt, so the conversion can be run without a window.

#include <cmath>

namespace TxLevel
{
constexpr int UnitsMax = 450;   // the stored value of 0.0 dB
constexpr double DbMin = -45.0;
constexpr double DbMax = 0.0;

inline int clampUnits(int units)
{
    return units < 0 ? 0 : (units > UnitsMax ? UnitsMax : units);
}

// stored value -> dB, one decimal exactly as the spin box holds it: 450 -> 0.0, 225 -> -22.5, 1 -> -44.9
inline double dbFromUnits(int units)
{
    return (clampUnits(units) - UnitsMax) / 10.0;
}

// dB -> stored value, rounded to the nearest tenth and held inside 0 .. 450
inline int unitsFromDb(double db)
{
    const double units = UnitsMax + std::floor(db * 10.0 + 0.5);
    if (units < 0.0) return 0;
    if (units > UnitsMax) return UnitsMax;
    return static_cast<int>(units);
}

// stored value -> linear sample gain: 450 -> 1.0, 390 (-6.0 dB) -> 0.501. 0 is -45 dB, not silence.
inline double gainFromUnits(int units)
{
    return std::pow(10.0, dbFromUnits(units) / 20.0);
}
} // namespace TxLevel
