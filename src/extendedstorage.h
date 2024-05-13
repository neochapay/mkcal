/*
  This file is part of the mkcal library.

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
  defines the ExtendedStorage interface.

  @author Tero Aho \<ext-tero.1.aho@nokia.com\>
  @author Alvaro Manera \<alvaro.manera@nokia.com \>
*/

#ifndef MKCAL_EXTENDEDSTORAGE_H
#define MKCAL_EXTENDEDSTORAGE_H

#include "mkcal_export.h"
#include "extendedcalendar.h"
#include "extendedstorageobserver.h"

#include <KCalendarCore/CalStorage>
#include <KCalendarCore/Calendar>

namespace KCalendarCore {
class Incidence;
}

class MkcalTool;
class tst_load;

namespace mKCal {

/**
  @brief
  This class provides a calendar storage interface.
  Every action on the storage can be synchronous or asynchronous,
  depending on the storage implementation. SqliteStorage is a
  synchronous implementation. 

  In any case, caller can use ExtendedStorageObserver to get
  notified about the action done.
*/
class MKCAL_EXPORT ExtendedStorage
    : public KCalendarCore::CalStorage, public KCalendarCore::Calendar::CalendarObserver
{
    Q_OBJECT

public:

    /**
      Action to be performed on save for deleted incidences.
    */
    enum DeleteAction {
        MarkDeleted,
        PurgeDeleted
    };

    /**
      A shared pointer to a ExtendedStorage
    */
    typedef QSharedPointer<ExtendedStorage> Ptr;

    /**
      Constructs a new ExtendedStorage object.

      @param cal is a pointer to a valid Calendar object.

      @warning Do not use storage as a global object, on closing the application
      it can dead lock. If you do so, be ready to destroy it manually before the
      application closes.

      @warning Once an Incidence has been added to the ExtendedStorage the UID
      cannot change. It is possible to do so through the API, but the internal
      hash tables will not be updated and hence the changes will not be tracked.
    */
    explicit ExtendedStorage(const ExtendedCalendar::Ptr &cal);

    /**
      Destructor.
    */
    virtual ~ExtendedStorage();

    /**
      @copydoc
      CalStorage::open()
    */
    virtual bool open() = 0;

    /**
      @copydoc
      CalStorage::load()
    */
    virtual bool load() = 0;

    /**
      Load all incidences sharing the same uid into the memory.

      @param uid is uid of the series
      @return true if the load was successful; false otherwise.
    */
    virtual bool load(const QString &uid) = 0;

    /**
      Load all incidences sharing the same uid into the memory.

      Deprecated call, equivalent to calling load(const QString &uid).

      @param uid is uid of the series
      @return true if the load was successful; false otherwise.
    */
    bool loadSeries(const QString &uid);

    /**
      Load incidence by uid/recid into the memory.

      This method is deprecated since it may populate calendars
      with orphaned exceptions, or recurring event without
      their exceptions.

      Use load(const QString &uid) which ensures for
      recurring incidences to also get their exceptions.

      @param uid is uid of incidence
      @param recurrenceid is recurrenceid of incidence
      @return true if the load was successful; false otherwise.
    */
    bool load(const QString &uid, const QDateTime &recurrenceId);

    /**
      Load incidences at given date into the memory. All incidences that
      happens within date, or starts / ends within date or span
      during date are loaded into memory. The time zone used to expand
      date into points in time is the time zone of the associated calendar.
      In addition, all recurring events are also loaded into memory since
      there is no way to know in advance if they will have occurrences
      intersecting date. Internally, recurring incidences and incidences of
      date are cached to avoid loading them several times.

      @param date date
      @return true if the load was successful; false otherwise.
    */
    virtual bool load(const QDate &date);

    /**
      Load incidences between given dates into the memory. start is inclusive,
      while end is exclusive. The same definitions and restrictions for loading
      apply as for load(const QDate &) method.

      @param start is the starting date
      @param end is the ending date, exclusive
      @return true if the load was successful; false otherwise.
    */
    virtual bool load(const QDate &start, const QDate &end) = 0;

    /**
      Load the incidence matching the given identifier. This method may be
      more fragile than load(uid, recid) though since the instanceIdentifier
      is not saved as is in the database.

      @param instanceIdentifier is an identifier returned by Incidence::instanceIdentifier()
      @return true if the load was successful; false otherwise.
    */
    virtual bool loadIncidenceInstance(const QString &instanceIdentifier);

    /**
      Remove from storage all incidences that have been previously
      marked as deleted and that matches the UID / RecID of the incidences
      in list. The action is performed immediately on database.

      @param list is the incidences to remove from the DB
      @return True on success, false otherwise.
     */
    virtual bool purgeDeletedIncidences(const KCalendarCore::Incidence::List &list) = 0;

    /**
      @copydoc
      CalStorage::save()
    */
    virtual bool save() = 0;

    /**
      This is an overload of save() method. When @deleteAction is
      PurgeDeleted, the deleted incidences are not marked as deleted but completely
      removed from the database and won't appear anymore when calling
      deletedIncidences().

      @param deleteAction the action to apply to deleted incidences
      @return True if successful; false otherwise
    */
    virtual bool save(DeleteAction deleteAction) = 0;

    /**
      @copydoc
      CalStorage::close()
    */
    virtual bool close();

    // Internal Calendar Listener Methods //

    /**
      @copydoc
      Calendar::CalendarObserver::calendarModified()
    */
    virtual void calendarModified(bool modified, KCalendarCore::Calendar *calendar) = 0;

    /**
      @copydoc
      Calendar::CalendarObserver::calendarIncidenceAdded()
    */
    virtual void calendarIncidenceAdded(const KCalendarCore::Incidence::Ptr &incidence) = 0;

    /**
      @copydoc
      Calendar::CalendarObserver::calendarIncidenceChanged()
    */
    virtual void calendarIncidenceChanged(const KCalendarCore::Incidence::Ptr &incidence) = 0;

    /**
      @copydoc
      Calendar::CalendarObserver::calendarIncidenceDeleted()
    */
    virtual void calendarIncidenceDeleted(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Calendar *calendar) = 0;

    /**
      @copydoc
      Calendar::CalendarObserver::calendarIncidenceAdditionCanceled()
    */
    virtual void calendarIncidenceAdditionCanceled(const KCalendarCore::Incidence::Ptr &incidence) = 0;

    // Synchronization Specific Methods //

    /**
      Get inserted incidences from storage.

      NOTE: time stamps assigned by KCalExtended are created during save().
      To obtain a time stamp that is guaranteed to not included recent changes,
      sleep for a second or increment the current time by a second.

      @param list inserted incidences
      @param after list only incidences inserted after or at given datetime
      @return true if execution was scheduled; false otherwise
    */
    virtual bool insertedIncidences(KCalendarCore::Incidence::List *list,
                                    const QDateTime &after = QDateTime()) = 0;

    /**
      Get modified incidences from storage.
      NOTE: if an incidence is both created and modified after the
      given time, it will be returned in insertedIncidences only, not here!

      @param list modified incidences
      @param after list only incidences modified after or at given datetime
      @return true if execution was scheduled; false otherwise
    */
    virtual bool modifiedIncidences(KCalendarCore::Incidence::List *list,
                                    const QDateTime &after = QDateTime()) = 0;

    /**
      Get deleted incidences from storage.

      @param list deleted incidences
      @param after list only incidences deleted after or at given datetime
      @return true if execution was scheduled; false otherwise
    */
    virtual bool deletedIncidences(KCalendarCore::Incidence::List *list,
                                   const QDateTime &after = QDateTime()) = 0;

    /**
      Get all incidences from storage.

      @param list notebook's incidences
      @return true if execution was scheduled; false otherwise
    */
    virtual bool allIncidences(KCalendarCore::Incidence::List *list) = 0;

    /**
      Get all incidences from storage that match key. Incidences are
      loaded into the associated ExtendedCalendar. More incidences than
      the listed ones in @param identifiers may be loaded into memory
      to ensure calendar consistency with respect to exceptions of
      recurring incidences.

      Matching is done on summary, description and location fields.

      The matching incidences are sorted by start dates before applying
      the @param limit. Since recurring incidences have occurrences later
      than their start date, they are not taken into account when
      counting the limit and all matching recurring events are always
      loaded.

      @param key can be any substring from the summary, the description or the location.
      @param identifiers optional, stores the instance identifiers of
             matching incidences.
      @param limit the maximum number of non-recurring incidences, unlimited by default
      @return true on success.
     */
    virtual bool search(const QString &key, QStringList *identifiers, int limit = 0) = 0;

    /**
      Get deletion time of incidence

      @param incidence incidence to check
      @return valid deletion time of incidence in UTC if incidence has been deleted otherwise QDateTime()
    */
    virtual QDateTime incidenceDeletedDate(const KCalendarCore::Incidence::Ptr &incidence) = 0;

    // Observer Specific Methods //

    /**
      @copydoc
      Calendar::addJournal()
    */
    bool addJournal(const KCalendarCore::Journal::Ptr &journal);
    void registerObserver(ExtendedStorageObserver *observer);

    /**
      Unregisters an Observer for this Storage.

      @param observer is a pointer to an Observer object that has been
      watching this Storage.

      @see registerObserver()
     */
    void unregisterObserver(ExtendedStorageObserver *observer);


    /**
      Standard trick to add virtuals later.

      @param id is any integer unique to this class which we will use to identify the method
             to be called.
      @param data is a pointer to some glob of data, typically a struct.
    */
    virtual void virtual_hook(int id, void *data) = 0;

protected:
    bool getLoadDates(const QDate &start, const QDate &end,
                      QDateTime *loadStart, QDateTime *loadEnd) const;

    void addLoadedRange(const QDate &start, const QDate &end) const;
    bool isRecurrenceLoaded() const;
    void setIsRecurrenceLoaded(bool loaded);

    void emitStorageModified(const QString &info);
    void emitStorageFinished(bool error, const QString &info);
    void emitStorageUpdated(const KCalendarCore::Incidence::List &added,
                            const KCalendarCore::Incidence::List &modified,
                            const KCalendarCore::Incidence::List &deleted);

private:
    //@cond PRIVATE
    Q_DISABLE_COPY(ExtendedStorage)
    class Private;
    Private *const d;
    //@endcond

    friend class ::MkcalTool;
    friend class ::tst_load;
};

}

#endif
