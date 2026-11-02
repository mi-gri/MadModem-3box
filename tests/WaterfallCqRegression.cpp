// SPDX-License-Identifier: GPL-3.0-or-later
// Waterfall resolution for the FT modes (DspEngine::setTargetBinHz) and the
// placement rules of the CQ calls on the waterfall (widgets/CqSpots.h).
#include "dsp/DspEngine.h"
#include "widgets/CqSpots.h"

#include <QObject>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

namespace {

constexpr int kRate = 48000;
constexpr double kMinHz = 100.0;
constexpr double kMaxHz = 3000.0;
constexpr int kColumns = 1024;

int columnOf(double hz)
{
    return static_cast<int>(std::lround((hz - kMinHz) / (kMaxHz - kMinHz) * (kColumns - 1)));
}

// The waterfall row after some seconds of two FT8-like tones (6.25 Hz apart) in noise.
std::vector<int> rowForTwoTones(double targetBinHz, double f1, double f2)
{
    DspEngine dsp;
    dsp.setTargetBinHz(targetBinHz);
    std::vector<int> last;
    QObject::connect(&dsp, &DspEngine::waterfallLineReady, [&last](const QVector<quint8> &line, double, double) {
        last.assign(line.begin(), line.end());
    });
    std::mt19937 rng(7);
    std::normal_distribution<float> noise(0.0f, 0.02f);
    AudioBlock block;
    block.sampleRate = kRate;
    const int blockSize = 4800;
    long long n = 0;
    for (int b = 0; b < 40; ++b) {   // 4 s
        block.samples.resize(blockSize);
        block.firstSampleIndex = n;   // contiguous, or AudioContinuity drops the FIFO
        for (int i = 0; i < blockSize; ++i, ++n) {
            const double t = static_cast<double>(n) / kRate;
            block.samples[i] = noise(rng) + 0.004f * static_cast<float>(std::sin(2 * M_PI * f1 * t) + std::sin(2 * M_PI * f2 * t));
        }
        dsp.processAudioBlock(block);
    }
    return last;
}

} // namespace

int main()
{
    bool ok = true;
    auto check = [&](bool condition, const char *name) {
        std::cout << (condition ? "PASS " : "FAIL ") << name << '\n';
        ok = condition && ok;
    };

    // --- waterfall resolution -------------------------------------------------
    const double f1 = 1500.0, f2 = 1500.0 + 6.25 * 2;   // two FT8 tones, two spacings apart
    for (double target : {0.0, 1.5}) {
        const std::vector<int> row = rowForTwoTones(target, f1, f2);
        const int c1 = columnOf(f1), c2 = columnOf(f2), mid = columnOf((f1 + f2) / 2);
        std::cout << "target " << target << " Hz:";
        for (int c = c1 - 4; c <= c2 + 4; ++c) std::cout << ' ' << (row.empty() ? -1 : row[c]);
        std::cout << '\n';
        if (target > 0.0) {
            check(!row.empty() && row[mid] + 40 < std::min(row[c1], row[c2]),
                  "FT8 waterfall: tones 12.5 Hz apart stand apart (dip between them)");
        }
    }

    // --- row rate, columns and row time ---------------------------------------
    {
        DspEngine dsp;
        dsp.setTargetBinHz(1.5);
        dsp.setTargetRowSeconds(0.3);
        dsp.setColumns(1600);
        int rows = 0;
        int width = 0;
        std::vector<qint64> times;
        QObject::connect(&dsp, &DspEngine::waterfallLineReady, [&](const QVector<quint8> &line, double, double) {
            ++rows;
            width = line.size();
        });
        QObject::connect(&dsp, &DspEngine::waterfallRowTime, [&](qint64 ms) { times.push_back(ms); });
        AudioBlock block;
        block.sampleRate = kRate;
        const qint64 startUtcNs = 1700000000000LL * 1000000LL;   // a capture time
        for (int b = 0; b < 100; ++b) {   // 10 s
            block.samples.fill(0.001f, 4800);
            block.firstSampleIndex = static_cast<qint64>(b) * 4800;
            block.firstSampleUtcNs = startUtcNs + static_cast<qint64>(b) * 100000000LL;
            dsp.processAudioBlock(block);
        }
        // 10 s minus one 0.68-s window, one row per 7 hops of 2048 samples (0.299 s)
        check(rows >= 28 && rows <= 32, "FT8 rows averaged to ~0.3 s");
        check(width == 1600, "one column per requested pixel");
        bool spaced = times.size() == static_cast<std::size_t>(rows) && times.size() > 2;
        for (std::size_t i = 1; spaced && i < times.size(); ++i)
            spaced = std::llabs(times[i] - times[i - 1] - 299) <= 2;
        check(spaced, "row times follow the capture clock, 0.3 s apart");
        check(!times.empty() && times.front() > 1700000000000LL && times.front() < 1700000001000LL,
              "row time is the window centre in capture UTC");
    }

    // --- CQ calls: fade -------------------------------------------------------
    check(cq_spots::fade(0, 30000) == 255 && cq_spots::fade(21000, 30000) == 255, "full opacity for the first 70 %");
    check(cq_spots::fade(25500, 30000) < 255 && cq_spots::fade(25500, 30000) > 80, "fades over the last 30 %");
    check(cq_spots::fade(30000, 30000) == 80, "about a third at the end");

    // --- CQ calls: rows -------------------------------------------------------
    {
        // three calls close together, two apart: three rows needed where they cluster
        std::vector<cq_spots::Box> boxes {{100, 60}, {110, 60}, {120, 60}, {400, 60}, {600, 60}};
        auto placed = cq_spots::stack(boxes, 800, 4, 2);
        bool allPlaced = true, noOverlap = true;
        for (std::size_t i = 0; i < placed.size(); ++i) {
            allPlaced = allPlaced && placed[i].row >= 0;
            for (std::size_t j = 0; j < i; ++j)
                if (placed[i].row == placed[j].row &&
                    placed[i].left < placed[j].left + boxes[j].width + 2 && placed[j].left < placed[i].left + boxes[i].width + 2)
                    noOverlap = false;
        }
        check(allPlaced, "all labels placed when rows suffice");
        check(noOverlap, "no two labels overlap in a row");
        check(placed[3].row == 0 && placed[4].row == 0, "lone calls stay on the bottom row");
    }
    {
        // more calls at one place than rows: the oldest (last) go, the newest stay
        std::vector<cq_spots::Box> boxes;
        for (int i = 0; i < 5; ++i) boxes.push_back({200, 60});
        auto placed = cq_spots::stack(boxes, 800, 3, 2);
        check(placed[0].row >= 0 && placed[1].row >= 0 && placed[2].row >= 0 && placed[3].row < 0 && placed[4].row < 0,
              "with too little room the oldest calls are left out");
    }
    {
        std::vector<cq_spots::Box> boxes {{5, 60}, {795, 60}};
        auto placed = cq_spots::stack(boxes, 800, 2, 2);
        check(placed[0].left == 0 && placed[1].left == 740, "labels are pushed inside the view at the edges");
    }

    return ok ? 0 : 1;
}
