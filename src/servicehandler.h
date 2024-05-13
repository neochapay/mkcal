#ifndef MKCAL_SERVICEHANDLER_H
#define MKCAL_SERVICEHANDLER_H
/*
  This file is part of the libmkcal library.

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

#include <KCalendarCore/Incidence>
#include <QObject>
#include "mkcal_export.h"
#include "servicehandlerif.h"

const QString defaultName = "DefaultInvitationPlugin";

class ServiceHandlerPrivate;

namespace mKCal {

/** Singleton class to get the exact handler (plugin) of the service
    In case of API with a notebook argument, the plugin to be used is
    determined calling `Notebook::pluginName()`.
*/
class MKCAL_EXPORT ServiceHandler : QObject
{
    Q_OBJECT
private:
    /** Constructor, is a singleton so you cannot do anything
      */
    ServiceHandler();
    ~ServiceHandler();

    ServiceHandlerPrivate *const d;

public:

    /** Error Codes that can be returned by the plugins */
    //Right now they are the same as defined in ServiceHandlerIf
    //But semantically it doesn't make sense that they are defined
    //there and at some point they might be different.
    enum ErrorCode {
        ErrorOk = 0,
        ErrorNoAccount,
        ErrorNotSupported,
        ErrorNoConnectivity,
        ErrorInvalidParameters,
        ErrorInternal
    };

    /** Obtain an instance of the ServiceHandler.
      @return The instance that handles all the services
      */
    static ServiceHandler &instance()
    {
        static ServiceHandler singleton;
        return singleton;
    }

    /** Icon
      @param serviceId the name of the service to use
      @return Icon
      */
    QString icon(const QString &serviceId);

    /** multiCalendar
      @param serviceId the name of the service to use
      @return True if multicalendar otherwise false
      */
    bool multiCalendar(const QString &serviceId);

    /** \brief In case of error, more detailed information can be provided
        Sometimes the true/false is not enough, so in case of false
        more details can be obtained.

        @param notebook notebook
        @param storage Pointer to the storage in use
        @return the ErrorCode of what happened
      */
    ServiceHandler::ErrorCode error() const;


    ///MultiCalendar services

    /** \brief List available Services
         There can be many available services. This method returns the ids of the plugins that handle
         those services.
         @note this id can be used in the Notebook creation to "attach" a notebook to a certain
         service.
         @return list of the ids of the plugins available
       */
    QStringList availableServices();

    /** \brief Get the Icon of a service based on the id of the plugin

      @return Path to the icon
      @see availableMulticalendarServices

      */
    QString icon(QString serviceId);

    /** \brief Get the Name to be shown on the UI of a service based on the id of the plugin

      @return Name of the service
      @see availableMulticalendarServices

      */
    QString uiName(QString serviceId);

    /** \brief Get the plugin object providing the service.

      @return the plugin object
      @see availableMulticalendarServices
      */
    ServiceInterface* service(const QString &serviceId);

signals:
    /** Monitors the progress of the download. The id is the return value got when download started */
    void downloadProgress(int id, int percentage);

    /** Informs that the download is over. The id is the return value got when download started */
    void downloadFinished(int id);

    /** Informs that the download is finished with errors. The id is the return value got when download started */
    void downloadError(int id, ErrorCode error);
};

}
#endif // MKCAL_SERVICEHANDLER_H
