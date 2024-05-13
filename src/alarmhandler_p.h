/*
  This file is part of the mkcal library.

  Copyright (c) 2023 Damien Caliste <dcaliste@free.fr>

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
  This file is part of the API for handling alarms.

  @author Damien Caliste \<dcaliste@free.fr\>
*/

#ifndef MKCAL_ALARMHANDLER_H
#define MKCAL_ALARMHANDLER_H

#include <KCalendarCore/Incidence>

#include "mkcal_export.h"

namespace mKCal {

/**
  @brief
  This class provides an interface to handle alarms.
*/
class AlarmHandler
{
protected:
    AlarmHandler() { }

    virtual ~AlarmHandler() { }

    /**
      Implement this method to provide incidences with alarms to the alarm handler.

      This method is called internally from setupAlarms() to get the
      information from a storage to setup alarms. Implementers of this
      class should guarantee that the series corresponding to uid is
      in memory when calling setupAlarms() with a non-empty uid, or calling
      setupAlarms() with several uids. If uid is empty, there is no guarantee
      given on the availablity of all incidences at the moment of the call.

      @param uid, when not empty, restrict the returned incidences to
             incidences sharing this UID.
      @returns a list of incidences with an alarm.
     */
    virtual KCalendarCore::Incidence::List incidencesWithAlarms(const QString &uid = QString()) = 0;
};
}

#endif
