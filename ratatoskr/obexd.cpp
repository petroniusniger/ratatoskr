/*==========================================================
 * Program : obexd.cpp                   Project : ratatoskr
 * Author  : Michael Zanetti, Ian L., Philippe Andersson
 * Date    : 2026-03-06
 * Version : 0.0.7
 * Notice  : (c) Original work by Michael Zanetti, Canonical
 *           Adapted by Ian L. and Philippe Andersson
 * License : GNU GPL v3 or later
 * Comment : OBEX daemon D-Bus interface for receiving files
 * Modification History:
 * - 2025-12-18 (0.0.1) : Adapted from ubtd-20.04.
 * - 2025-12-25 (0.0.3) : Changed to systemBus() for AppArmor compliance.
 * - 2026-01-15 (0.0.4) : Fixed to sessionBus() with service discovery.
 * - 2026-01-20 (0.0.5) : Fixed logging message for OBEX service activation.
 * - 2026-02-12 (0.0.6) : Added comments for service discovery and retry logic.
 * - 2026-03-06 (0.0.7) : Added transfer confirmation (accept/reject) before saving.
 *========================================================*/

#include "obexd.h"

#include "obexagent.h"

#include <QDBusReply>
#include <QThread>
#include <QDBusConnectionInterface>

Obexd::Obexd(QObject *parent) :
    QAbstractListModel(parent),
    m_dbus(QDBusConnection::sessionBus()),
    m_manager(nullptr)
{
    qDebug() << "creating agent on dbus";

    m_agent = new ObexAgent(this);

    qDebug() << "discovering obex service";
    QString serviceName = findObexService();
    
    if (serviceName.isEmpty()) {
        qWarning() << "OBEX service not found. File transfers will not work.";
        qWarning() << "Try starting obexd manually: systemctl --user start obex.service";
    } else {
        qDebug() << "found OBEX service at:" << serviceName;
        m_manager = new QDBusInterface(serviceName, "/org/bluez/obex", "org.bluez.obex.AgentManager1", m_dbus, this);
        registerAgent(serviceName);
    }

    connect(m_agent, &ObexAgent::authorized, this, &Obexd::newTransfer);

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    qDebug() << "have entries:" << dir.entryList() << dir.path();
    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QDir::Files)) {
        Transfer* t = new Transfer("/completed", fileInfo.absoluteFilePath(), this);
        m_transfers.append(t);
    }
}

int Obexd::rowCount(const QModelIndex &parent) const
{
    return m_transfers.count();
}

QVariant Obexd::data(const QModelIndex &index, int role) const
{
    //qDebug() << "asked for data" << index.row() << role << m_transfers.count();
    switch(role) {
    case RoleFilename:
        return m_transfers.at(index.row())->filename();
    case RoleFilePath:
        return m_transfers.at(index.row())->filePath();
    case RoleSize:
        return m_transfers.at(index.row())->total();
    case RoleTransferred:
        return m_transfers.at(index.row())->transferred();
    case RoleStatus:
        return m_transfers.at(index.row())->status();
    case RoleDate:
        return QFileInfo(m_transfers.at(index.row())->filePath() + "/" + m_transfers.at(index.row())->filename()).created();
    }
    return QVariant();
}

QHash<int, QByteArray> Obexd::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleFilename, "filename");
    roles.insert(RoleFilePath, "filePath");
    roles.insert(RoleSize, "size");
    roles.insert(RoleTransferred, "transferred");
    roles.insert(RoleStatus, "status");
    roles.insert(RoleDate, "date");
    return roles;
}

void Obexd::deleteFile(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    Transfer* t = m_transfers.takeAt(index);
    QFile f(t->filePath() + "/" + t->filename());
    f.remove();
    endRemoveRows();
}

void Obexd::newTransfer(const QString &path)
{
    qDebug() << "new transfer" << path;

    QString targetPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir dir(targetPath);
    if (!dir.exists()) {
        dir.mkpath(targetPath);
    }

    Transfer *t = new Transfer(path, targetPath, this);
    connect(t, &Transfer::totalChanged, this, &Obexd::transferProgress);
    connect(t, &Transfer::transferredChanged, this, &Obexd::transferProgress);
    connect(t, &Transfer::statusChanged, this, &Obexd::transferStatusChanged);

    beginInsertRows(QModelIndex(), m_transfers.count(), m_transfers.count());
    m_transfers.append(t);
    endInsertRows();
    qDebug() << "added to model";

    m_pendingAcceptPaths[path] = targetPath + "/" + t->filename();
    emit transferNeedsConfirmation(path, t->filename(), t->total());
}

void Obexd::acceptTransfer(const QString &path)
{
    if (!m_pendingAcceptPaths.contains(path)) {
        qWarning() << "acceptTransfer: unknown path" << path;
        return;
    }
    QString targetFile = m_pendingAcceptPaths.take(path);
    qDebug() << "accepting transfer" << path << "->" << targetFile;
    m_agent->accept(path, targetFile);
}

void Obexd::rejectTransfer(const QString &path)
{
    if (!m_pendingAcceptPaths.contains(path)) {
        qWarning() << "rejectTransfer: unknown path" << path;
        return;
    }
    m_pendingAcceptPaths.remove(path);
    qDebug() << "rejecting transfer" << path;

    // Remove transfer from model
    for (int i = 0; i < m_transfers.count(); ++i) {
        if (m_transfers.at(i)->path() == path) {
            beginRemoveRows(QModelIndex(), i, i);
            Transfer *t = m_transfers.takeAt(i);
            t->deleteLater();
            endRemoveRows();
            break;
        }
    }

    m_agent->reject(path);
}

void Obexd::transferProgress()
{
    Transfer *t = qobject_cast<Transfer*>(sender());
    QModelIndex idx = index(m_transfers.indexOf(t));
    emit dataChanged(idx, idx, QVector<int>() << RoleTransferred << RoleSize);
}

void Obexd::transferStatusChanged()
{
    Transfer *t = qobject_cast<Transfer*>(sender());
    QModelIndex idx = index(m_transfers.indexOf(t));
    emit dataChanged(idx, idx, QVector<int>() << RoleStatus);
}

QString Obexd::findObexService()
{
    QDBusConnectionInterface *interface = m_dbus.interface();
    if (!interface) {
        qWarning() << "Failed to get D-Bus interface";
        return QString();
    }

    // Multi-stage OBEX service discovery strategy:
    // Ubuntu Touch's AppArmor and D-Bus activation can cause timing/permission issues
    
    // Stage 1: Direct name lookup (fastest, works if service already running)
    if (interface->isServiceRegistered("org.bluez.obex")) {
        qDebug() << "Found OBEX service by name: org.bluez.obex";
        return "org.bluez.obex";
    }

    // Stage 2: D-Bus activation with timing workaround
    qDebug() << "OBEX service not found, attempting D-Bus activation";
    QDBusReply<void> activationReply = interface->startService("org.bluez.obex");
    if (activationReply.isValid()) {
        // CRITICAL: 500ms delay allows bluez-obexd to fully initialize
        // Without this, service may appear registered but not respond to calls
        QThread::msleep(500);
        if (interface->isServiceRegistered("org.bluez.obex")) {
            qDebug() << "OBEX service activated successfully";
            return "org.bluez.obex";
        }
    } else {
        qDebug() << "D-Bus activation failed:" << activationReply.error().message();
    }

    // Stage 3: Dynamic address lookup (fallback for unnamed/confined services)
    // Some confinement scenarios result in services with dynamic addresses like ":1.234"
    qDebug() << "Searching for OBEX service by interface";
    QDBusReply<QStringList> servicesReply = interface->registeredServiceNames();
    if (servicesReply.isValid()) {
        for (const QString &service : servicesReply.value()) {
            if (service.startsWith(":")) {
                QDBusInterface testInterface(service, "/org/bluez/obex", "org.bluez.obex.AgentManager1", m_dbus);
                if (testInterface.isValid()) {
                    qDebug() << "Found OBEX service at dynamic address:" << service;
                    return service;
                }
            }
        }
    }

    return QString();
}

bool Obexd::registerAgent(const QString &serviceName, int attempt)
{
    const int maxAttempts = 3;
    // Exponential backoff: 0ms, 1s, 3s
    // Works around race conditions in bluez-obexd initialization
    const int delays[] = {0, 1000, 3000};

    if (attempt > 0) {
        qDebug() << "Retrying agent registration, attempt" << (attempt + 1) << "of" << maxAttempts;
        QThread::msleep(delays[attempt]);
    }

    qDebug() << "registering agent on obexd-server";
    QDBusReply<void> reply = m_manager->call("RegisterAgent", qVariantFromValue(QDBusObjectPath(DBUS_ADAPTER_AGENT_PATH)));
    
    if (reply.isValid()) {
        qDebug() << "Agent registered successfully";
        return true;
    }

    qWarning() << "Error registering agent (attempt" << (attempt + 1) << "):" << reply.error().message();

    if (attempt < maxAttempts - 1) {
        return registerAgent(serviceName, attempt + 1);
    }

    qCritical() << "Failed to register agent after" << maxAttempts << "attempts";
    return false;
}

