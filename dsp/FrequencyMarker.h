#ifndef FREQUENCYMARKER_H
#define FREQUENCYMARKER_H

#include <QColor>
#include <QString>
#include <QMetaType>

/**
 * @brief Describes one important frequency marker for the waterfall.
 *
 * Purpose:
 * - Mark mode-specific tones.
 * - Keep marker data outside the waterfall drawing code.
 * - Allow each modem/demodulator to expose its own visual hints.
 */
struct FrequencyMarker
{
    double frequencyHz = 0.0;
    QString label;
    QColor color = QColor(220, 0, 0);
    bool dashed = false;
    int width = 2;

    /**
     * Width of the signal in Hz. 0 draws a single line at frequencyHz; more
     * draws the band the signal occupies, from frequencyHz upwards (the base
     * tone, as FT8/FT4/Q65 count it) or, when centred, around frequencyHz.
     */
    double bandwidthHz = 0.0;
    bool centred = false;
};

Q_DECLARE_METATYPE(FrequencyMarker)

#endif // FREQUENCYMARKER_H
