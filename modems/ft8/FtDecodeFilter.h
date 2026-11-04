// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>

// The "Only CQ/RR73/73" filter of the FT decode table: the stations that are
// free to be called - those calling CQ and those ending a QSO with RR73 or 73.
// A line addressed to my call always passes.
namespace FtDecodeFilter {
inline bool passesCqEnd(const QString &message, const QString &myCall)
{
    const QStringList words = message.trimmed().toUpper().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (words.isEmpty()) return false;
    if (words.first() == QStringLiteral("CQ")) return true;
    const QString me = myCall.trimmed().toUpper();
    if (!me.isEmpty() && words.first() == me) return true;
    return words.last() == QStringLiteral("RR73") || words.last() == QStringLiteral("73");
}

// Kind of a decode line, used for its JTDX colour.
enum class LineKind { Standard, Cq, End, MyCall };

inline LineKind lineKind(const QString &message, const QString &myCall)
{
    const QStringList words = message.trimmed().toUpper().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (words.isEmpty()) return LineKind::Standard;
    const QString me = myCall.trimmed().toUpper();
    if (!me.isEmpty() && words.first() == me) return LineKind::MyCall;
    if (words.first() == QStringLiteral("CQ")) return LineKind::Cq;
    if (words.last() == QStringLiteral("RR73") || words.last() == QStringLiteral("73")) return LineKind::End;
    return LineKind::Standard;
}

// The "worked stations" filter: hide a station already in the log.
// days = 0 hides every worked station; days > 0 hides only those whose last
// QSO is at most that many days ago (older contacts are shown again).
inline bool hiddenAsWorked(bool worked, const QDateTime &lastQsoUtc, int days, const QDateTime &nowUtc)
{
    if (!worked) return false;
    if (days <= 0) return true;
    return lastQsoUtc.isValid() && lastQsoUtc.secsTo(nowUtc) <= qint64{days} * 86400;
}
} // namespace FtDecodeFilter
