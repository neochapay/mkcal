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

#include "alarmhandler_p.h"
#include "logging_p.h"

using namespace mKCal;

#include <KCalendarCore/Todo>
using namespace KCalendarCore;

#ifdef TIMED_SUPPORT
# include <timed-qt6/interface.h>
# include <timed-qt6/event-declarations.h>
# include <timed-qt6/exception.h>
# include <QtCore/QMap>
# include <QtDBus/QDBusReply>
using namespace Maemo;
static const QLatin1String RESET_ALARMS_CMD("invoker --type=generic -n /usr/bin/mkcaltool --reset-alarms");
#endif

static bool cancelAlarms(const QSet<QPair<QString, QString>> &uids)
{
#if defined(TIMED_SUPPORT)
    Timed::Interface timed;
    if (!timed.isValid()) {
        qCWarning(lcMkcal) << "cannot clear alarms,"
                 << "timed interface is not valid" << timed.lastError();
        return false;
    }

    QMap<QString, QVariant> query;
    query["APPLICATION"] = "libextendedkcal";
    QDBusReply<QList<QVariant> > reply = timed.query_sync(query);
    if (!reply.isValid()) {
        qCWarning(lcMkcal) << "cannot get alarm cookies" << timed.lastError();
        return false;
    }
    QList<uint> cookiesAll;
    for (const QVariant &variant : reply.value()) {
        cookiesAll.append(variant.toUInt());
    }
    QDBusReply<QMap<uint, QMap<QString,QString> >> attributes = timed.get_attributes_by_cookies_sync(cookiesAll);
    if (!attributes.isValid()) {
        qCWarning(lcMkcal) << "cannot get alarm attributes" << timed.lastError();
        return false;
    }
    QList<uint> cookiesDoomed;
    const QMap<uint, QMap<QString,QString> > map = attributes.value();
    for (QMap<uint, QMap<QString,QString> >::ConstIterator it = map.constBegin();
         it != map.constEnd(); it++) {
        const QString uid = it.value()["uid"];
        qCDebug(lcMkcal) << "removing alarm" << it.key() << uid;
        cookiesDoomed.append(it.key());
    }

    if (!cookiesDoomed.isEmpty()) {
        QDBusReply<QList<uint>> reply = timed.cancel_events_sync(cookiesDoomed);
        if (!reply.isValid() || !reply.value().isEmpty()) {
            qCWarning(lcMkcal) << "cannot remove alarms" << cookiesDoomed;
        }
    }
#endif
    return true;
}

#if defined(TIMED_SUPPORT)
static QDateTime getNextOccurrence(const Recurrence *recurrence,
                                   const QDateTime &start,
                                   const QSet<QDateTime> &recurrenceIds)
{
    QDateTime match = start;
    if (!recurrence->recursAt(start) || recurrenceIds.contains(start)) {
        do {
            match = recurrence->getNextDateTime(match);
        } while (match.isValid() && recurrenceIds.contains(match));
    }
    return match;
}

static void addAlarms(Timed::Event::List *events, const Incidence &incidence,
                      const QDateTime &laterThan)
{
    if (incidence.status() == Incidence::StatusCanceled || laterThan.isNull()) {
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();
    const Alarm::List alarms = incidence.alarms();
    foreach (const Alarm::Ptr alarm, alarms) {
        if (!alarm->enabled()) {
            continue;
        }

        QDateTime preTime = laterThan;
        if (incidence.recurs() && alarm->startOffset().asSeconds() < 0) {
            // by construction for recurring events, laterThan is the time of the
            // actual next occurrence, so one need to remove the alarm offset.
            preTime = preTime.addSecs(alarm->startOffset().asSeconds());
        }

        // nextTime() is returning time strictly later than its argument.
        QDateTime alarmTime = alarm->nextTime(preTime.addSecs(-1), true);
        if (!alarmTime.isValid()) {
            continue;
        }

        if (now.addSecs(60) > alarmTime) {
            // don't allow alarms within the current minute -> take next alarm if so
            alarmTime = alarm->nextTime(preTime.addSecs(60), true);
            if (!alarmTime.isValid()) {
                continue;
            }
        }
        Timed::Event &e = events->append();
        e.setUserModeFlag();
        e.setMaximalTimeoutSnoozeCounter(2);
        e.setTicker(alarmTime.toUTC().toMSecsSinceEpoch());
        // The code'll crash (=exception) iff the content is empty. So
        // we have to check here.
        QString s;

        s = incidence.summary();
        // Timed braindeath: Required field, BUT if empty, it asserts
        if (s.isEmpty()) {
            s = ' ';
        }
        e.setAttribute("TITLE", s);
        e.setAttribute("PLUGIN", "libCalendarReminder");
        e.setAttribute("APPLICATION", "libextendedkcal");
        //e.setAttribute( "translation", "organiser" );
        // This really has to exist or code is badly broken
        Q_ASSERT(!incidence.uid().isEmpty());
        e.setAttribute("uid", incidence.uid());
#ifndef QT_NO_DEBUG_OUTPUT //Helps debuggin
        e.setAttribute("alarmtime", alarmTime.toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate));
#endif
        if (!incidence.location().isEmpty()) {
            e.setAttribute("location", incidence.location());
        }
        if (incidence.recurs()) {
            e.setAttribute("recurs", "true");
            Timed::Event::Action &a = e.addAction();
            a.runCommand(QString("%1 %2 %3")
                         .arg(RESET_ALARMS_CMD)
                         .arg(incidence.uid()));
            a.whenServed();
        }

        // TODO - consider this how it should behave for recurrence
        if ((incidence.type() == Incidence::TypeTodo)) {
            const Todo *todo = static_cast<const Todo*>(&incidence);

            if (todo->hasDueDate()) {
                e.setAttribute("time", todo->dtDue(true).toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate));
            }
            e.setAttribute("type", "todo");
        } else if (incidence.dtStart().isValid()) {
            QDateTime eventStart;

            if (incidence.recurs()) {
                // assuming alarms not later than event start
                eventStart = incidence.recurrence()->getNextDateTime(alarmTime.addSecs(-60));
            } else {
                eventStart = incidence.dtStart();
            }
            e.setAttribute("time", eventStart.toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate));
            e.setAttribute("startDate", eventStart.toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate));
            if (incidence.endDateForStart(eventStart).isValid()) {
                e.setAttribute("endDate", incidence.endDateForStart(eventStart).toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate));
            }
            e.setAttribute("type", "event");
        }

        if (incidence.hasRecurrenceId()) {
            e.setAttribute("recurrenceId", incidence.recurrenceId().toString(Qt::ISODate));
        }

        if (alarm->type() == Alarm::Procedure) {
            QString prog = alarm->programFile();
            if (!prog.isEmpty()) {
                Timed::Event::Action &a = e.addAction();
                a.runCommand(prog + " " + alarm->programArguments());
                a.whenFinalized();
            }
        } else {
            e.setReminderFlag();
            e.setAlignedSnoozeFlag();
        }
    }
}
#endif
