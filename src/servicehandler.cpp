/*
  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <QtCore/QHash>
#include <QtCore/QPluginLoader>
#include <QtCore/QStringList>
#include <QtCore/QDir>

#include "servicehandler.h"
#include "servicehandlerif.h"
#include "invitationhandlerif.h"
#include "logging_p.h"

using namespace mKCal;
using namespace KCalendarCore;

class ServiceHandlerPrivate
{
public:
    QHash<QString, InvitationHandlerInterface *> mPlugins;
    QHash<QString, ServiceInterface *> mServices;

    bool mLoaded;
    int mDownloadId;
    ServiceHandler::ErrorCode mError;

    void loadPlugins();
    InvitationHandlerInterface* invitationPlugin(const QString &pluginName);

    ServiceHandlerPrivate();
};

ServiceHandlerPrivate::ServiceHandlerPrivate() : mLoaded(false), mDownloadId(0),
    mError(ServiceHandler::ErrorOk)
{
}

void ServiceHandlerPrivate::loadPlugins()
{
    QString pluginPath = QLatin1String(qgetenv("MKCAL_PLUGIN_DIR"));
    if (pluginPath.isEmpty())
        pluginPath = QLatin1String(MKCALPLUGINDIR);
    QDir pluginsDir(pluginPath);
    qCDebug(lcMkcal) << "LOADING !!!! Plugin directory" << pluginsDir.path();

    foreach (const QString &fileName, pluginsDir.entryList(QDir::Files)) {
        qCDebug(lcMkcal) << "Loading service handler plugin" << fileName;
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();

        if (!loader.isLoaded()) {
            qCDebug(lcMkcal) << "Failed to load plugin:" << loader.errorString();
        }
        if (plugin) {
            if (ServiceInterface *interface = qobject_cast<ServiceInterface *>(plugin)) {
                mServices.insert(interface->serviceName(), interface);
                qCDebug(lcMkcal) << "Loaded service:" << interface->serviceName();
            }
            if (InvitationHandlerInterface *interface = qobject_cast<InvitationHandlerInterface *>(plugin)) {
                mPlugins.insert(interface->pluginName(), interface);
                qCDebug(lcMkcal) << "Loaded plugin:" << interface->pluginName();
            }
        }  else {
            qCDebug(lcMkcal) << fileName << " Not a plugin";
        }
    }

    mLoaded = true;
}

InvitationHandlerInterface* ServiceHandlerPrivate::invitationPlugin(const QString &pluginName)
{
    if (!mLoaded)
        loadPlugins();

    QHash<QString, InvitationHandlerInterface *>::const_iterator i = mPlugins.find(pluginName);
    if (i != mPlugins.end()) {
        return i.value();
    } else if ((i = mPlugins.find(defaultName)) != mPlugins.end()) {
        return i.value();
    } else {
        return nullptr;
    }
}

ServiceHandler::ServiceHandler()
    : d(new ServiceHandlerPrivate())
{
}

QString ServiceHandler::icon(const QString &serviceId)
{
    ServiceInterface *plugin = service(serviceId);
    if (!plugin) {
        plugin = service(defaultName);
    }

    return plugin ? plugin->icon() : QString();
}

bool ServiceHandler::multiCalendar(const QString &serviceId)
{
    ServiceInterface *plugin = service(serviceId);
    if (!plugin) {
        plugin = service(defaultName);
    }

    d->mError = ErrorOk;
    if (plugin) {
        bool res = plugin->multiCalendar();
        d->mError = (ServiceHandler::ErrorCode) plugin->error(); //Right now convert directly
        return res;
    } else {
        return false;
    }
}

QStringList ServiceHandler::availableServices()
{
    if (!d->mLoaded)
        d->loadPlugins();
    QStringList result;

    foreach (ServiceInterface *service, d->mServices) {
        result.append(service->serviceName());
    }

    return result;
}

QString ServiceHandler::icon(QString serviceId)
{
    ServiceInterface *plugin = service(serviceId);
    return plugin ? plugin->icon() : QString();
}

QString ServiceHandler::uiName(QString serviceId)
{
    ServiceInterface *plugin = service(serviceId);
    return plugin ? plugin->uiName() : QString();
}

ServiceInterface* ServiceHandler::service(const QString &serviceId)
{
    if (!d->mLoaded)
        d->loadPlugins();

    QHash<QString, ServiceInterface *>::const_iterator i = d->mServices.find(serviceId);

    if (i != d->mServices.end()) {
        return i.value();
    } else {
        return nullptr;
    }
}

ServiceHandler::ErrorCode ServiceHandler::error() const
{
    return d->mError;
}

ServiceHandler::~ServiceHandler()
{
    delete d;
}
