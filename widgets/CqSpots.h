// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// CQ calls on the waterfall: where their labels go, and how they fade.
//
// The callsigns of stations calling CQ are written at the bottom of the
// waterfall, over the audio frequency each was decoded on, for a time the
// operator sets. Ported from the JTDX 3box CQ calls on the waterfall
// (cqspots.h), so both programs place them alike:
//   - an entry fades out over the last 30 % of its time
//   - labels of signals close together would print over each other (an FT8
//     signal is 44 Hz wide, a call is wider than that on any screen), so they
//     are stacked in rows above the waterfall's bottom edge, packed so that as
//     many as possible fit; where they do not all fit, the oldest calls are
//     left unwritten (their frequency brackets are still drawn, and the plot
//     counts them: "+N")
//
// Plain C++ without Qt, so the rules can be run without a window.

#include <algorithm>
#include <cstddef>
#include <vector>

namespace cq_spots
{
int const max_rows {8};   // label rows at most, above the bracket line

// opacity, 0..255, of an entry of the given age: full for the first 70 % of
// its time, then down to about a third of it by the time it goes
inline int fade(long long age_ms, long long duration_ms)
{
    if (duration_ms <= 0) return 255;
    double const f {static_cast<double>(age_ms) / static_cast<double>(duration_ms)};
    if (f <= 0.7) return 255;
    if (f >= 1.0) return 80;
    return 255 - static_cast<int>((f - 0.7) / 0.3 * 175.0 + 0.5);
}

struct Box    // a label to be placed: where its signal is, and how wide the label is, in pixels
{
    int centre;
    int width;
};

struct Slot   // where it went: row 0 is the lowest; row -1 means no row had room
{
    int row;
    int left;
};

// Places as many of the labels as the rows (0 .. rows-1) hold, each keeping
// gap pixels clear of its neighbours in the row. A label is centred on its
// signal, and pushed inwards where that would take it past the edges of the
// view (0 .. view_w). The boxes come in order of importance, the newest first;
// the result is in the same order, and row -1 marks a label for which there
// was no room.
//
// Two ways of placing them are tried and the one that places more is kept:
//   - from the left: the rows are filled in order of frequency, which uses the
//     fewest rows there can be. Where there is not enough room, the oldest of
//     the labels in the way of the one that did not fit is left out and the
//     rest laid out again, so it is the oldest calls that go; those left out
//     are then tried once more in whatever room is free;
//   - by age: the newest first, each in the lowest row that has room.
namespace detail
{
inline std::vector<int> rows_by_age(std::vector<int> const &left, std::vector<int> const &right, int rows, int gap)
{
    std::size_t const n {left.size()};
    std::vector<int> row_of(n, -1);
    for (std::size_t i = 0; i < n; ++i)
        for (int r = 0; r < rows && row_of[i] < 0; ++r) {
            bool free_here {true};
            for (std::size_t j = 0; j < i && free_here; ++j)
                if (row_of[j] == r && left[i] < right[j] + gap && left[j] < right[i] + gap) free_here = false;
            if (free_here) row_of[i] = r;
        }
    return row_of;
}

inline std::vector<int> rows_from_left(std::vector<int> const &left, std::vector<int> const &right, int rows, int gap)
{
    std::size_t const n {left.size()};
    std::vector<bool> alive(n, true);
    std::vector<std::size_t> order;
    std::vector<int> row_of(n, -1);
    for (;;) {
        order.clear();
        for (std::size_t i = 0; i < n; ++i) if (alive[i]) order.push_back(i);
        std::stable_sort(order.begin(), order.end(), [&left](std::size_t a, std::size_t b) { return left[a] < left[b]; });
        std::vector<int> row_end(static_cast<std::size_t>(rows), 0);   // right edge of the last label of each row
        std::vector<bool> row_used(static_cast<std::size_t>(rows), false);
        std::fill(row_of.begin(), row_of.end(), -1);
        std::size_t misfit {n};
        for (std::size_t k = 0; k < order.size() && misfit == n; ++k) {
            std::size_t const i {order[k]};
            bool found {false};
            for (std::size_t r = 0; r < row_end.size() && !found; ++r)
                if (!row_used[r] || left[i] >= row_end[r] + gap) {
                    row_used[r] = true;
                    row_end[r] = right[i];
                    row_of[i] = static_cast<int>(r);
                    found = true;
                }
            if (!found) misfit = i;
        }
        if (misfit == n) break;
        // the oldest (highest index) of those that overlap the misfit, itself included, goes
        std::size_t victim {misfit};
        for (std::size_t j : order)
            if (j > victim && left[j] < right[misfit] + gap && left[misfit] < right[j] + gap) victim = j;
        alive[victim] = false;
    }
    // those left out, newest first, into any room that is free
    for (std::size_t i = 0; i < n; ++i)
        for (int r = 0; r < rows && row_of[i] < 0; ++r) {
            bool free_here {true};
            for (std::size_t j = 0; j < n && free_here; ++j)
                if (j != i && row_of[j] == r && left[i] < right[j] + gap && left[j] < right[i] + gap) free_here = false;
            if (free_here) row_of[i] = r;
        }
    return row_of;
}

inline std::size_t count_placed(std::vector<int> const &row_of)
{
    return static_cast<std::size_t>(std::count_if(row_of.begin(), row_of.end(), [](int r) { return r >= 0; }));
}
} // namespace detail

inline std::vector<Slot> stack(std::vector<Box> const &boxes, int view_w, int rows, int gap)
{
    std::size_t const n {boxes.size()};
    std::vector<Slot> placed(n, Slot {-1, 0});
    std::vector<int> left(n);
    std::vector<int> right(n);
    for (std::size_t i = 0; i < n; ++i) {
        left[i] = std::max(0, std::min(boxes[i].centre - boxes[i].width / 2, view_w - boxes[i].width));
        right[i] = left[i] + boxes[i].width;
        placed[i].left = left[i];
    }
    if (rows <= 0) return placed;
    std::vector<int> const from_left {detail::rows_from_left(left, right, rows, gap)};
    std::vector<int> const by_age {detail::rows_by_age(left, right, rows, gap)};
    std::vector<int> const &best {detail::count_placed(by_age) > detail::count_placed(from_left) ? by_age : from_left};
    for (std::size_t i = 0; i < n; ++i) placed[i].row = best[i];
    return placed;
}
} // namespace cq_spots
