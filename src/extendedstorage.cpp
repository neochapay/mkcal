/*
  This file is part of the mkcal library.

  Copyright (c) 2002,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  Contact: Alvaro Manera <alvaro.manera@nokia.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the ExtendedStorage abstract base class.

  @brief
  An abstract base class that provides a calendar storage interface.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/
#include "extendedstorage.h"
#include "extendedstorageobserver.h"
#include "alarmhandler_p.h"
#include "logging_p.h"

#include <KCalendarCore/Exceptions>
#include <KCalendarCore/Calendar>
using namespace KCalendarCore;

using namespace mKCal;

struct Range
{
    Range(const QDate &start, const QDate &end)
        : mStart(start), mEnd(end) { }

    bool contains(const QDate &at) const
    {
        return at.isValid()
            && (mStart.isNull() || at >= mStart)
            && (mEnd.isNull() || at <= mEnd);
    }

    QDate mStart, mEnd;
};

// Range a is strictly before range b.
bool operator<(const Range &a, const Range &b)
{
    return a.mEnd.isValid() && b.mStart.isValid() && a.mEnd < b.mStart;
}
// Date a is strictly before range b.
bool operator<(const QDate &at, const Range &range)
{
    return at.isNull() || (range.mStart.isValid() && at < range.mStart);
}

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class mKCal::ExtendedStorage::Private: public AlarmHandler
{
public:
    Private(ExtendedStorage *storage)
        : mStorage(storage)
        , mIsRecurrenceLoaded(false)
    {}

    ~Private()
    {}

    ExtendedStorage *mStorage;
    QList<Range> mRanges;
    bool mIsRecurrenceLoaded;
    QList<ExtendedStorageObserver *> mObservers;
    bool clear();

    Incidence::List incidencesWithAlarms(const QString &uid);
};

bool ExtendedStorage::Private::clear()
{
    mRanges.clear();
    mIsRecurrenceLoaded = false;
    return true;
}

Incidence::List ExtendedStorage::Private::incidencesWithAlarms(const QString &uid)
{
    Incidence::List list;

    // The assumption on wether the incidences are already in memory
    // or not is explained in AlarmHandler::incidencesWithAlarms() documentation
    // and is reminded here for completeness.
    if (uid.isEmpty()) {
        // There is no guarantee that the calendar contains all incidences.
        Incidence::List all;
        mStorage->allIncidences(&all);
        for (Incidence::List::ConstIterator it = all.constBegin();
             it != all.constEnd(); it++) {
            // Recurring incidences may not have alarms but their exception may.
            if ((*it)->hasEnabledAlarms() || (*it)->recurs()) {
                list.append(*it);
            }
        }
    } else {
        // This case is called when modifying (insertion, update or deletion)
        // one or several incidences. The series is guaranteed to be already
        // in memory.
        Incidence::Ptr parent = mStorage->calendar()->incidence(uid);
        if (!parent) {
            return list;
        }
        if (parent->hasEnabledAlarms()) {
            list.append(parent);
        }
        for (const Incidence::Ptr &exception : mStorage->calendar()->instances(parent)) {
            if (exception->hasEnabledAlarms() || parent->hasEnabledAlarms()) {
                list.append(exception);
            }
        }
    }
    return list;
}
//@endcond

ExtendedStorage::ExtendedStorage(const ExtendedCalendar::Ptr &cal)
    : CalStorage(cal),
      d(new ExtendedStorage::Private(this))
{
    cal->registerObserver(this);
}

ExtendedStorage::~ExtendedStorage()
{
    calendar()->unregisterObserver(this);
    delete d;
}

bool ExtendedStorage::close()
{
    return d->clear();
}

bool ExtendedStorage::getLoadDates(const QDate &start, const QDate &end,
                                   QDateTime *loadStart, QDateTime *loadEnd) const
{
    loadStart->setDate(start);   // may be null if start is not valid
    loadEnd->setDate(end);   // may be null if end is not valid

    // Check the need to load from db.
    for (const Range &loadedRange : d->mRanges) {
        bool startIsIn = loadedRange.contains(loadStart->date())
            || (loadedRange.mStart.isNull() && loadStart->date().isNull());
        bool endIsIn = loadedRange.contains(loadEnd->date().addDays(-1))
            || (loadedRange.mEnd.isNull() && loadEnd->date().isNull());
        if (startIsIn && endIsIn) {
            return false;
        } else if (startIsIn) {
            loadStart->setDate(loadedRange.mEnd.addDays(1));
        } else if (endIsIn) {
            loadEnd->setDate(loadedRange.mStart);
        }
    }
    if (loadStart->isValid() && loadEnd->isValid() && *loadStart >= *loadEnd) {
        return false;
    }

    if (loadStart->isValid()) {
        loadStart->setTimeZone(calendar()->timeZone());
    }
    if (loadEnd->isValid()) {
        loadEnd->setTimeZone(calendar()->timeZone());
    }

    qCDebug(lcMkcal) << "get load dates" << start << end << *loadStart << *loadEnd;

    return true;
}

void ExtendedStorage::addLoadedRange(const QDate &start, const QDate &end) const
{
    qCDebug(lcMkcal) << "set load dates" << start << end;

    Range range(start, end.addDays(-1));
    QList<Range>::Iterator it = d->mRanges.begin();
    while (it != d->mRanges.end()) {
        if (range < *it) {
            d->mRanges.insert(it, range);
            return;
        } else if (it->contains(end)) {
            if (start < *it) {
                it->mStart = start;
            }
            return;
        } else if (start < *it) {
            it = d->mRanges.erase(it);
        } else if (it->contains(start)) {
            range.mStart = it->mStart;
            it = d->mRanges.erase(it);
        } else {
            it++;
        }
    }
    d->mRanges.append(range);
}

bool ExtendedStorage::isRecurrenceLoaded() const
{
    return d->mIsRecurrenceLoaded;
}

void ExtendedStorage::setIsRecurrenceLoaded(bool loaded)
{
    d->mIsRecurrenceLoaded = loaded;
}

bool ExtendedStorage::loadSeries(const QString &uid)
{
    qCWarning(lcMkcal) << "deprecated call to loadSeries(), use load() instead.";
    return load(uid);
}

bool ExtendedStorage::load(const QString &uid, const QDateTime &recurrenceId)
{
    Q_UNUSED(recurrenceId);

    qCWarning(lcMkcal) << "deprecated call to load(uid, recid), use load(uid) instead.";
    return load(uid);
}

bool ExtendedStorage::loadIncidenceInstance(const QString &instanceIdentifier)
{
    QString uid;
    // At the moment, from KCalendarCore, if the instance is an exception,
    // the instanceIdentifier will ends with yyyy-MM-ddTHH:mm:ss[Z|[+|-]HH:mm]
    // This is tested in tst_loadIncidenceInstance() to ensure that any
    // future breakage would be properly detected.
    if (instanceIdentifier.endsWith('Z')) {
        uid = instanceIdentifier.left(instanceIdentifier.length() - 20);
    } else if (instanceIdentifier.length() > 19
               && instanceIdentifier[instanceIdentifier.length() - 9] == 'T') {
        uid = instanceIdentifier.left(instanceIdentifier.length() - 19);
    } else if (instanceIdentifier.length() > 25
               && instanceIdentifier[instanceIdentifier.length() - 3] == ':') {
        uid = instanceIdentifier.left(instanceIdentifier.length() - 25);
    } else {
        uid = instanceIdentifier;
    }

    // Even if we're looking for a specific incidence instance, we load all
    // the series for recurring event, to avoid orphaned exceptions in the
    // calendar or recurring events without their exceptions.
    return load(uid);
}

bool ExtendedStorage::load(const QDate &date)
{
    return date.isValid() && load(date, date.addDays(1));
}

void ExtendedStorageObserver::storageModified(ExtendedStorage *storage,
                                              const QString &info)
{
    Q_UNUSED(storage);
    Q_UNUSED(info);
}

void ExtendedStorageObserver::storageFinished(ExtendedStorage *storage,
                                              bool error, const QString &info)
{
    Q_UNUSED(storage);
    Q_UNUSED(error);
    Q_UNUSED(info);
}

void ExtendedStorageObserver::storageUpdated(ExtendedStorage *storage,
                                             const KCalendarCore::Incidence::List &added,
                                             const KCalendarCore::Incidence::List &modified,
                                             const KCalendarCore::Incidence::List &deleted)
{
    Q_UNUSED(storage);
    Q_UNUSED(added);
    Q_UNUSED(modified);
    Q_UNUSED(deleted);
}

void ExtendedStorage::registerObserver(ExtendedStorageObserver *observer)
{
    if (!d->mObservers.contains(observer)) {
        d->mObservers.append(observer);
    }
}

void ExtendedStorage::unregisterObserver(ExtendedStorageObserver *observer)
{
    d->mObservers.removeAll(observer);
}

void ExtendedStorage::emitStorageModified(const QString &info)
{
    foreach (ExtendedStorageObserver *observer, d->mObservers) {
        observer->storageModified(this, info);
    }
}

void ExtendedStorage::emitStorageFinished(bool error, const QString &info)
{
    foreach (ExtendedStorageObserver *observer, d->mObservers) {
        observer->storageFinished(this, error, info);
    }
}

void ExtendedStorage::emitStorageUpdated(const KCalendarCore::Incidence::List &added,
                                         const KCalendarCore::Incidence::List &modified,
                                         const KCalendarCore::Incidence::List &deleted)
{
    foreach (ExtendedStorageObserver *observer, d->mObservers) {
        observer->storageUpdated(this, added, modified, deleted);
    }
}
