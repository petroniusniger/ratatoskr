/*==========================================================
 * Program : obexagent.cpp               Project : ratatoskr
 * Author  : Michael Zanetti, Ian L., Philippe Andersson
 * Date    : 2026-03-06
 * Version : 0.0.3
 * Notice  : (c) Original work by Michael Zanetti, Canonical
 *           Adapted by Ian L. and Philippe Andersson
 * License : GNU GPL v3 or later
 * Comment : OBEX agent implementation
 * Modification History:
 * - 2025-12-18 (0.0.1) : Adapted from ubtd-20.04.
 * - 2026-02-12 (0.0.2) : Added comments explaining D-Bus deferred reply pattern.
 * - 2026-03-06 (0.0.3) : Added reject() for graceful transfer cancellation.
 *========================================================*/

#include "obexagent.h"

#include <QStandardPaths>

ObexAgent::ObexAgent(QObject *parent) :
    QObject(parent),
    m_dbus(QDBusConnection::sessionBus())
{
    qDebug() << "registering agent";
    m_agent = new ObexAgentAdaptor(this);
    if(!m_dbus.registerObject(DBUS_ADAPTER_AGENT_PATH, this))
        qCritical() << "Couldn't register agent at" << DBUS_ADAPTER_AGENT_PATH;

}

void ObexAgent::accept(const QString &path, const QString &fileName)
{
    if (!m_pendingRequests.contains(path)) {
        return;
    }
    QDBusMessage msg = m_pendingRequests.take(path);
    QDBusConnection::sessionBus().send(msg.createReply(fileName));
}

void ObexAgent::reject(const QString &path)
{
    if (!m_pendingRequests.contains(path)) {
        return;
    }
    QDBusMessage msg = m_pendingRequests.take(path);
    QDBusConnection::sessionBus().send(
        msg.createErrorReply("org.bluez.obex.Error.Rejected", "Transfer rejected by user"));
}

QString ObexAgent::AuthorizePush(const QDBusObjectPath &transfer)
{
    qDebug() << "authorize called" <<  transfer.path();

    // D-Bus deferred reply pattern: we need to return immediately but respond later
    // setDelayedReply(true) tells D-Bus this method will reply asynchronously
    setDelayedReply(true);
    
    // Store the original D-Bus message so we can send the reply later via accept()
    m_pendingRequests[transfer.path()] = message();
    
    // Emit signal via QTimer to avoid Qt event loop re-entrancy issues
    // Direct emit could cause problems if handlers call back into D-Bus
    QTimer::singleShot(0, [this, transfer]() { emit authorized(transfer.path());});
    
    qDebug() << "returning";
    // Empty return is required by D-Bus protocol for async handlers
    return QString();
}

