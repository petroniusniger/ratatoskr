/*==========================================================
 * Program : obextransfer.cpp            Project : ratatoskr
 * Author  : Philippe Andersson + Copilot CLI.
 * Date    : 2025-12-19
 * Version : 0.01
 * Notice  : (c) Les Ateliers du Heron, 2025
 * License : GNU GPL v3 or later
 * Comment : OBEX outgoing file transfer service using D-Bus.
 * Modification History:
 * - 2025-12-19 (0.01) : Initial release.
 *========================================================*/

#include "obextransfer.h"
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QFileInfo>
#include <QDebug>

// OBEXTransfer implementation

OBEXTransfer::OBEXTransfer(const QString &sessionPath, const QString &transferPath, QObject *parent)
  : QObject(parent)
  , m_sessionPath(sessionPath)
  , m_transferPath(transferPath)
  , m_transfer(nullptr)
  , m_size(0)
  , m_transferred(0)
  , m_status(StatusQueued)
{
  QDBusConnection bus = QDBusConnection::sessionBus();
  m_transfer = new QDBusInterface(
    "org.bluez.obex",
    m_transferPath,
    "org.bluez.obex.Transfer1",
    bus,
    this
  );

  bus.connect(
    "org.bluez.obex",
    m_transferPath,
    "org.freedesktop.DBus.Properties",
    "PropertiesChanged",
    this,
    SLOT(onPropertiesChanged(QString,QVariantMap,QStringList))
  );

  loadProperties();
}

OBEXTransfer::~OBEXTransfer()
{
  delete m_transfer;
}

void OBEXTransfer::loadProperties()
{
  QVariant filename = getProperty("Filename");
  if (filename.isValid()) {
    m_filename = filename.toString();
    emit filenameChanged();
  }

  QVariant size = getProperty("Size");
  if (size.isValid()) {
    m_size = size.toLongLong();
    emit sizeChanged();
  }

  QVariant transferred = getProperty("Transferred");
  if (transferred.isValid()) {
    m_transferred = transferred.toLongLong();
    emit transferredChanged();
  }

  QVariant status = getProperty("Status");
  if (status.isValid()) {
    Status newStatus = statusFromString(status.toString());
    if (m_status != newStatus) {
      m_status = newStatus;
      emit statusChanged();
      
      if (m_status == StatusComplete) {
        emit completed();
      } else if (m_status == StatusError) {
        emit failed("Transfer failed");
      }
    }
  }
}

QVariant OBEXTransfer::getProperty(const QString &property)
{
  if (!m_transfer) {
    return QVariant();
  }

  QDBusInterface props(
    "org.bluez.obex",
    m_transferPath,
    "org.freedesktop.DBus.Properties",
    QDBusConnection::sessionBus()
  );

  QDBusReply<QVariant> reply = props.call("Get", "org.bluez.obex.Transfer1", property);
  if (!reply.isValid()) {
    return QVariant();
  }

  return reply.value();
}

OBEXTransfer::Status OBEXTransfer::statusFromString(const QString &statusStr)
{
  if (statusStr == "queued") return StatusQueued;
  if (statusStr == "active") return StatusActive;
  if (statusStr == "complete") return StatusComplete;
  if (statusStr == "error") return StatusError;
  return StatusError;
}

int OBEXTransfer::progress() const
{
  if (m_size == 0) return 0;
  return (m_transferred * 100) / m_size;
}

void OBEXTransfer::cancel()
{
  if (!m_transfer) {
    return;
  }

  m_transfer->call("Cancel");
}

void OBEXTransfer::onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
  Q_UNUSED(invalidatedProperties);

  if (interface != "org.bluez.obex.Transfer1") {
    return;
  }

  if (changedProperties.contains("Transferred")) {
    qint64 transferred = changedProperties["Transferred"].toLongLong();
    if (m_transferred != transferred) {
      m_transferred = transferred;
      emit transferredChanged();
      emit progressChanged();
    }
  }

  if (changedProperties.contains("Status")) {
    Status newStatus = statusFromString(changedProperties["Status"].toString());
    if (m_status != newStatus) {
      m_status = newStatus;
      emit statusChanged();
      
      if (m_status == StatusComplete) {
        emit completed();
      } else if (m_status == StatusError) {
        emit failed("Transfer failed");
      }
    }
  }
}

// OBEXTransferService implementation

OBEXTransferService::OBEXTransferService(QObject *parent)
  : QObject(parent)
  , m_obexClient(nullptr)
{
  QDBusConnection bus = QDBusConnection::sessionBus();
  if (!bus.isConnected()) {
    emit errorOccurred("Cannot connect to session D-Bus");
    return;
  }

  m_obexClient = new QDBusInterface(
    "org.bluez.obex",
    "/org/bluez/obex",
    "org.bluez.obex.Client1",
    bus,
    this
  );

  if (!m_obexClient->isValid()) {
    emit errorOccurred("Cannot access OBEX client service");
  }
}

OBEXTransferService::~OBEXTransferService()
{
  qDeleteAll(m_activeTransfers);
  delete m_obexClient;
}

QString OBEXTransferService::createSession(const QString &deviceAddress)
{
  if (!m_obexClient || !m_obexClient->isValid()) {
    emit errorOccurred("OBEX client not available");
    return QString();
  }

  QVariantMap args;
  args["Target"] = "OPP";

  QDBusReply<QDBusObjectPath> reply = m_obexClient->call("CreateSession", deviceAddress, args);
  if (!reply.isValid()) {
    emit errorOccurred("Failed to create session: " + reply.error().message());
    return QString();
  }

  return reply.value().path();
}

void OBEXTransferService::removeSession(const QString &sessionPath)
{
  if (!m_obexClient || sessionPath.isEmpty()) {
    return;
  }

  m_obexClient->call("RemoveSession", QVariant::fromValue(QDBusObjectPath(sessionPath)));
}

OBEXTransfer* OBEXTransferService::sendFile(const QString &deviceAddress, const QString &filePath)
{
  if (!m_obexClient || !m_obexClient->isValid()) {
    emit errorOccurred("OBEX client not available");
    return nullptr;
  }

  QFileInfo fileInfo(filePath);
  if (!fileInfo.exists()) {
    emit errorOccurred("File does not exist: " + filePath);
    return nullptr;
  }

  QString sessionPath = createSession(deviceAddress);
  if (sessionPath.isEmpty()) {
    return nullptr;
  }

  QDBusInterface session(
    "org.bluez.obex",
    sessionPath,
    "org.bluez.obex.ObjectPush1",
    QDBusConnection::sessionBus()
  );

  if (!session.isValid()) {
    emit errorOccurred("Failed to access ObjectPush interface");
    removeSession(sessionPath);
    return nullptr;
  }

  QDBusReply<QDBusObjectPath> reply = session.call("SendFile", filePath);
  if (!reply.isValid()) {
    emit errorOccurred("Failed to send file: " + reply.error().message());
    removeSession(sessionPath);
    return nullptr;
  }

  QString transferPath = reply.value().path();
  OBEXTransfer *transfer = new OBEXTransfer(sessionPath, transferPath, this);
  
  connect(transfer, &OBEXTransfer::completed, this, [this, transfer, sessionPath]() {
    removeSession(sessionPath);
    m_activeTransfers.removeOne(transfer);
    transfer->deleteLater();
  });
  
  connect(transfer, &OBEXTransfer::failed, this, [this, transfer, sessionPath](const QString &error) {
    Q_UNUSED(error);
    removeSession(sessionPath);
    m_activeTransfers.removeOne(transfer);
    transfer->deleteLater();
  });

  m_activeTransfers.append(transfer);
  emit transferStarted(transfer);

  return transfer;
}

void OBEXTransferService::cancelTransfer(OBEXTransfer *transfer)
{
  if (!transfer) {
    return;
  }

  transfer->cancel();
}
