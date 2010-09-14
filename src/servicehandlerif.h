#ifndef SERVICEHANDLERIF_H
#define SERVICEHANDLERIF_H
/*
  This file is part of the kcal library.

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

#include <QtCore/QtPlugin>
#include <Qt/qicon.h>

#include "notebook.h"

class QString;

/**
  @file
  This file defines the common Interface to be inherited by all processes
  that handle service information

  @author Alvaro Manera \<alvaro.manera@nokia.com\>
*/

/** \brief Interface implemented by plugins for handling services.

    */

class ServiceInterface {

public:

    /** \brief returns icon of service.
        @return icon.
    */
    virtual QIcon icon() const = 0;

    /** \brief is this service supporting multiple calendars.
        @return true is service supports multiple calendars otherwise false.
    */
    virtual  bool multiCalendar() const = 0;

    /** \brief returns the email address that is currently configured in the service,
	it can be different per account.
        @param notebook pointer to the notebook that we want to share
        @return email address of service
    */
    virtual QString emailAddress(const mKCal::Notebook::Ptr &notebook) const = 0;

    /** \brief returns the display name of account of service.
        @param notebook pointer to the notebook that we want to share
        @return display name of account of service
    */
    virtual QString displayName(const mKCal::Notebook::Ptr &notebook) const = 0;

    /** \brief sttart the download of an attachment.
        @param uri uri of attachment to be downloaded
	@return True if OK, false otherwise.
    */
    virtual bool downloadAttachment(const QString &uri) const = 0;

    /** \brief Share notebook.
        @param notebook pointer to the notebook that we want to share
        @param sharedWith email address or phone number of users
        @return True if OK, false otherwise.
    */
    virtual bool shareNotebook(const mKCal::Notebook::Ptr &notebook, const QStringList &sharedWith) = 0;

    /** \brief Returns list of emails, phones# of the persons that a notebook is shared with.
        @param notebook pointer to the notebook
        @return list of email addresses or phone numbers
    */
    virtual QStringList sharedWith(const mKCal::Notebook::Ptr &notebook) const = 0;

    /** \brief The name of this service.
        It should be a uniq name specifying which service to use
        The service name is stored in the Calendars table (pluginname).
        @return The name of the service.
     */
    virtual QString serviceName() const = 0;

    virtual ~ServiceInterface() { }
};

Q_DECLARE_INTERFACE(ServiceInterface,
                    "com.nokia.Organiser.ServiceInterface/1.0")

#endif // SERVICEHANDLERIF_H