// SPDX-License-Identifier: GPL-3.0-or-later
#include "audio/TxLevel.h"
#include <cmath>
#include <iostream>

int main()
{
    bool ok = true;
    const auto check = [&ok](bool pass, const char *what) {
        std::cout << (pass ? "PASS " : "FAIL ") << what << '\n';
        ok &= pass;
    };
    check(TxLevel::dbFromUnits(450) == 0.0 && TxLevel::dbFromUnits(225) == -22.5 &&
          std::fabs(TxLevel::dbFromUnits(1) + 44.9) < 1e-9, "units -> dB");
    check(TxLevel::unitsFromDb(0.0) == 450 && TxLevel::unitsFromDb(-22.5) == 225 &&
          TxLevel::unitsFromDb(-44.9) == 1 && TxLevel::unitsFromDb(-45.0) == 0, "dB -> units");
    check(TxLevel::unitsFromDb(-6.04) == 390 && TxLevel::unitsFromDb(-6.06) == 389, "dB rounded to a tenth");
    check(TxLevel::unitsFromDb(3.0) == 450 && TxLevel::unitsFromDb(-60.0) == 0 &&
          TxLevel::dbFromUnits(999) == 0.0 && TxLevel::dbFromUnits(-5) == -45.0, "held inside 0..450");
    bool roundTrip = true;
    for (int units = 0; units <= TxLevel::UnitsMax; ++units)
        roundTrip &= TxLevel::unitsFromDb(TxLevel::dbFromUnits(units)) == units;
    check(roundTrip, "every stored value survives the spin box");
    check(TxLevel::gainFromUnits(450) == 1.0 && std::fabs(TxLevel::gainFromUnits(390) - 0.501187) < 1e-5 &&
          std::fabs(TxLevel::gainFromUnits(0) - 0.0056234) < 1e-6, "units -> linear gain");
    return ok ? 0 : 1;
}
