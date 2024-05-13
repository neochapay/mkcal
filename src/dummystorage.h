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
#ifndef DUMMYSTORAGE_H
#define DUMMYSTORAGE_H

#include "extendedstorage.h"
#include "extendedcalendar.h"

/**
 * This module provides a simple storage abstraction which contains
 * exactly nothing. It is only inteded to use for testing purposes
 */

class MKCAL_EXPORT DummyStorage : public mKCal::ExtendedStorage
{
public:
    DummyStorage(const mKCal::ExtendedCalendar::Ptr &cal) : mKCal::ExtendedStorage(cal)
    {
    }

    void calendarModified(bool, KCalendarCore::Calendar *)
    {
    }
    void calendarIncidenceAdded(const KCalendarCore::Incidence::Ptr &)
    {
    }
    void calendarIncidenceChanged(const KCalendarCore::Incidence::Ptr &)
    {
    }
    void calendarIncidenceDeleted(const KCalendarCore::Incidence::Ptr &, const KCalendarCore::Calendar *)
    {
    }
    void calendarIncidenceAdditionCanceled(const KCalendarCore::Incidence::Ptr &)
    {
    }
    bool open()
    {
        return true;
    }
    bool load()
    {
        return true;
    }
    bool save()
    {
        return true;
    }
    bool save(DeleteAction)
    {
        return true;
    }
    bool close()
    {
        return true;
    }
    bool load(const QString &)
    {
        return true;
    }
    bool load(const QDate &, const QDate &)
    {
        return true;
    }
    bool purgeDeletedIncidences(const KCalendarCore::Incidence::List &, const QString &)
    {
        return true;
    }
    bool insertedIncidences(KCalendarCore::Incidence::List *, const QDateTime &, const QString &)
    {
        return true;
    }
    bool modifiedIncidences(KCalendarCore::Incidence::List *, const QDateTime &, const QString &)
    {
        return true;
    }
    bool deletedIncidences(KCalendarCore::Incidence::List *, const QDateTime &, const QString &)
    {
        return true;
    }
    bool allIncidences(KCalendarCore::Incidence::List *, const QString &)
    {
        return true;
    }
    bool search(const QString &, QStringList *, int)
    {
        return true;
    }
    QDateTime incidenceDeletedDate(const KCalendarCore::Incidence::Ptr &)
    {
        return QDateTime();
    }
    void virtual_hook(int, void *)
    {
        return;
    }
};

#endif /* DUMMYSTORAGE_H */
