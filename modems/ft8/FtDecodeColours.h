#pragma once

// FT4/FT8 decode-table colours as in JTDX (displaytext.cpp):
//  - the TEXT colour tells the kind of message: CQ and QSO ends (RR73/73)
//    black, lines to my call red, all other traffic grey;
//  - the BACKGROUND tells what the station would be new for, checked in the
//    JTDX order: new DXCC, new DXCC on this band, new grid, new grid on this
//    band, new call, new call on this band/mode. A station worked before keeps
//    a plain background and is struck through.
// Other traffic (reports between other stations) keeps a plain row; only its
// call cell carries the "new" colour, like JTDX's marker column.
// In a dark theme the light JTDX colours are darkened as JTDX's own dark set.

#include <QColor>
#include <QDateTime>

#include <cmath>

namespace FtDecodeColours {

enum class Novelty { Unknown, None, Worked, NewCallBand, NewCall, NewGridBand, NewGrid, NewDxccBand, NewDxcc };

struct LogStatus {
    bool known = false;
    bool callWorked = false;
    bool callWorkedBandMode = false;
    bool dxccKnown = false;      // a DXCC entity could be derived from the call
    bool dxccWorked = false;
    bool dxccWorkedBand = false;
    bool gridKnown = false;      // the station's locator is known
    bool gridWorked = false;
    bool gridWorkedBand = false;
    QDateTime lastQsoUtc;        // latest QSO with this call (any band/mode)
};

struct Options {
    bool newDxcc = true;         // JTDX defaults: DXCC and call on, grid off
    bool newGrid = false;
    bool newCall = true;
};

inline Novelty novelty(const LogStatus &s, const Options &o)
{
    if (!s.known) return Novelty::Unknown;
    if (o.newDxcc && s.dxccKnown && !s.dxccWorked) return Novelty::NewDxcc;
    if (o.newDxcc && s.dxccKnown && !s.dxccWorkedBand) return Novelty::NewDxccBand;
    if (o.newGrid && s.gridKnown && !s.gridWorked) return Novelty::NewGrid;
    if (o.newGrid && s.gridKnown && !s.gridWorkedBand) return Novelty::NewGridBand;
    if (o.newCall && !s.callWorked) return Novelty::NewCall;
    if (o.newCall && !s.callWorkedBandMode) return Novelty::NewCallBand;
    return s.callWorked ? Novelty::Worked : Novelty::None;
}

inline bool isNew(Novelty n)
{
    return n != Novelty::Unknown && n != Novelty::None && n != Novelty::Worked;
}

// Relative luminance (sRGB, WCAG) and the contrast ratio of two colours.
inline double luminance(const QColor &c)
{
    auto lin = [](double v) { return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
    return 0.2126 * lin(c.redF()) + 0.7152 * lin(c.greenF()) + 0.0722 * lin(c.blueF());
}

inline double contrast(const QColor &a, const QColor &b)
{
    const double la = luminance(a), lb = luminance(b);
    return (qMax(la, lb) + 0.05) / (qMin(la, lb) + 0.05);
}

inline bool isDark(const QColor &c) { return c.lightness() < 128; }

// JTDX's dark set is the light colour at about 60 % brightness.
inline QColor forTheme(const QColor &light, bool darkTheme)
{
    if (!darkTheme) return light;
    return QColor::fromHsv(light.hsvHue() < 0 ? 0 : light.hsvHue(), light.hsvSaturation(),
                           qRound(light.value() * 0.62));
}

// Keep the configured text colour, but never let it vanish in the
// background: grey/black text is mirrored on dark backgrounds, and anything
// still too faint falls back to black or white.
inline QColor readable(const QColor &text, const QColor &background)
{
    QColor fg = text;
    if (isDark(background) && fg.hslSaturation() < 60 && fg.lightness() < 128) {
        fg = QColor::fromHsl(qMax(0, fg.hslHue()), fg.hslSaturation(), 255 - fg.lightness());
    }
    if (contrast(fg, background) < 3.0) {
        const QColor black(0, 0, 0), white(255, 255, 255);
        fg = contrast(black, background) >= contrast(white, background) ? black : white;
    }
    return fg;
}

} // namespace FtDecodeColours
