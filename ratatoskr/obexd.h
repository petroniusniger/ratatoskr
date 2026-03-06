/*==========================================================
 * Program : obexd.h                     Project : ratatoskr
 * Author  : Michael Zanetti, Ian L., Philippe Andersson
 * Date    : 2026-03-06
 * Version : 0.0.5
 * Notice  : (c) Original work by Michael Zanetti, Canonical
 *           Adapted by Ian L. and Philippe Andersson
 * License : GNU GPL v3 or later
 * Comment : OBEX daemon D-Bus interface header
 * Modification History:
 * - 2025-12-18 (0.0.1) : Adapted from ubtd-20.04.
 * - 2026-01-15 (0.0.4) : Added service discovery and session bus.
 * - 2026-03-06 (0.0.5) : Added confirmation signal and accept/reject slots.
 *========================================================*/

#ifndef OBEXD_H
#define OBEXD_H

#include <QAbstractListModel>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QHash>

#include "transfer.h"

class ObexAgent;

class Obexd : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        RoleFilename,
        RoleFilePath,
        RoleSize,
        RoleTransferred,
        RoleStatus,
        RoleDate
    };

    explicit Obexd(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    void deleteFile(int index);
    void acceptTransfer(const QString &path);
    void rejectTransfer(const QString &path);

signals:
    void transferNeedsConfirmation(const QString &path, const QString &filename, quint64 size);

private slots:
    void newTransfer(const QString &path);

    void transferProgress();
    void transferStatusChanged();

private:
    QString findObexService();
    bool registerAgent(const QString &serviceName, int attempt = 0);

    ObexAgent *m_agent;
    QDBusConnection m_dbus;
    QDBusInterface *m_manager;

    QList<Transfer*> m_transfers;
    QHash<QString, QString> m_pendingAcceptPaths;
};

#endif // OBEXD_H
