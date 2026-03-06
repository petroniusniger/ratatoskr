/*==========================================================
 * Program : obexagent.h                 Project : ratatoskr
 * Author  : Michael Zanetti, Ian L., Philippe Andersson
 * Date    : 2026-03-06
 * Version : 0.0.2
 * Notice  : (c) Original work by Michael Zanetti, Canonical
 *           Adapted by Ian L. and Philippe Andersson
 * License : GNU GPL v3 or later
 * Comment : OBEX agent header
 * Modification History:
 * - 2025-12-18 (0.0.1) : Adapted from ubtd-20.04.
 * - 2026-03-06 (0.0.2) : Added reject() for graceful transfer cancellation.
 *========================================================*/

#ifndef OBEXAGENT_H
#define OBEXAGENT_H

#include <QObject>
#include <QDBusObjectPath>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusAbstractAdaptor>

#define DBUS_ADAPTER_AGENT_PATH "/test/agent"

#include "obexagentadaptor.h"

class ObexAgent : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    explicit ObexAgent(QObject *parent = 0);

    void accept(const QString & path, const QString &fileName);
    void reject(const QString &path);

signals:
    void authorized(const QString &path);

public slots:
    QString AuthorizePush(const QDBusObjectPath &transfer);
    void Cancel() {}
    void Release() {}

private:
    QDBusConnection m_dbus;
    ObexAgentAdaptor *m_agent;

    QHash<QString, QDBusMessage> m_pendingRequests;
};

#endif // OBEXAGENT_H
