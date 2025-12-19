/*==========================================================
 * Program : bluetoothmanager.cpp        Project : ratatoskr
 * Author  : Philippe Andersson + Copilot CLI.
 * Date    : 2025-12-19
 * Version : 0.01
 * Notice  : (c) Les Ateliers du Heron, 2025
 * License : GNU GPL v3 or later
 * Comment : Central Bluetooth management service using Qt D-Bus.
 * Modification History:
 * - 2025-12-19 (0.01) : Initial release.
 *========================================================*/

#include "bluetoothmanager.h"
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDebug>

BluetoothManager::BluetoothManager(QObject *parent)
  : QObject(parent)
  , m_bluezManager(nullptr)
  , m_adapter(nullptr)
  , m_powered(false)
  , m_discoverable(false)
{
}

BluetoothManager::~BluetoothManager()
{
  delete m_adapter;
  delete m_bluezManager;
}

bool BluetoothManager::initialize()
{
  QDBusConnection bus = QDBusConnection::systemBus();
  if (!bus.isConnected()) {
    emit errorOccurred("Cannot connect to system D-Bus");
    return false;
  }

  m_bluezManager = new QDBusInterface(
    "org.bluez",
    "/",
    "org.freedesktop.DBus.ObjectManager",
    bus,
    this
  );

  if (!m_bluezManager->isValid()) {
    emit errorOccurred("Cannot access BlueZ service");
    return false;
  }

  QDBusMessage reply = m_bluezManager->call("GetManagedObjects");
  if (reply.type() == QDBusMessage::ErrorMessage) {
    emit errorOccurred("Cannot get managed objects: " + reply.errorMessage());
    return false;
  }

  const QDBusArgument arg = reply.arguments().at(0).value<QDBusArgument>();
  arg.beginMap();
  while (!arg.atEnd()) {
    QString path;
    arg.beginMapEntry();
    arg >> path;
    
    QVariantMap interfaces;
    arg >> interfaces;
    arg.endMapEntry();

    if (interfaces.contains("org.bluez.Adapter1")) {
      m_adapterPath = path;
      break;
    }
  }
  arg.endMap();

  if (m_adapterPath.isEmpty()) {
    emit errorOccurred("No Bluetooth adapter found");
    return false;
  }

  m_adapter = new QDBusInterface(
    "org.bluez",
    m_adapterPath,
    "org.bluez.Adapter1",
    bus,
    this
  );

  if (!m_adapter->isValid()) {
    emit errorOccurred("Cannot access Bluetooth adapter");
    return false;
  }

  bus.connect(
    "org.bluez",
    m_adapterPath,
    "org.freedesktop.DBus.Properties",
    "PropertiesChanged",
    this,
    SLOT(onPropertiesChanged(QString,QVariantMap,QStringList))
  );

  return loadAdapterProperties();
}

bool BluetoothManager::loadAdapterProperties()
{
  QVariant powered = getAdapterProperty("Powered");
  if (powered.isValid()) {
    m_powered = powered.toBool();
  }

  QVariant discoverable = getAdapterProperty("Discoverable");
  if (discoverable.isValid()) {
    m_discoverable = discoverable.toBool();
  }

  QVariant address = getAdapterProperty("Address");
  if (address.isValid()) {
    m_adapterAddress = address.toString();
    emit adapterAddressChanged();
  }

  QVariant name = getAdapterProperty("Name");
  if (name.isValid()) {
    m_adapterName = name.toString();
    emit adapterNameChanged();
  }

  return true;
}

QVariant BluetoothManager::getAdapterProperty(const QString &property)
{
  if (!m_adapter) {
    return QVariant();
  }

  QDBusInterface props(
    "org.bluez",
    m_adapterPath,
    "org.freedesktop.DBus.Properties",
    QDBusConnection::systemBus()
  );

  QDBusReply<QVariant> reply = props.call("Get", "org.bluez.Adapter1", property);
  if (!reply.isValid()) {
    qWarning() << "Failed to get property" << property << ":" << reply.error().message();
    return QVariant();
  }

  return reply.value();
}

void BluetoothManager::setAdapterProperty(const QString &property, const QVariant &value)
{
  if (!m_adapter) {
    return;
  }

  QDBusInterface props(
    "org.bluez",
    m_adapterPath,
    "org.freedesktop.DBus.Properties",
    QDBusConnection::systemBus()
  );

  QDBusMessage reply = props.call("Set", "org.bluez.Adapter1", property, QVariant::fromValue(QDBusVariant(value)));
  if (reply.type() == QDBusMessage::ErrorMessage) {
    emit errorOccurred("Failed to set " + property + ": " + reply.errorMessage());
  }
}

void BluetoothManager::setPowered(bool powered)
{
  if (m_powered != powered) {
    setAdapterProperty("Powered", powered);
    m_powered = powered;
    emit poweredChanged();
  }
}

void BluetoothManager::setDiscoverable(bool discoverable)
{
  if (m_discoverable != discoverable) {
    setAdapterProperty("Discoverable", discoverable);
    m_discoverable = discoverable;
    emit discoverableChanged();
  }
}

void BluetoothManager::startDiscovery()
{
  if (!m_adapter) {
    emit errorOccurred("Adapter not initialized");
    return;
  }

  QDBusMessage reply = m_adapter->call("StartDiscovery");
  if (reply.type() == QDBusMessage::ErrorMessage) {
    emit errorOccurred("Failed to start discovery: " + reply.errorMessage());
  }
}

void BluetoothManager::stopDiscovery()
{
  if (!m_adapter) {
    return;
  }

  m_adapter->call("StopDiscovery");
}

void BluetoothManager::onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
  Q_UNUSED(invalidatedProperties);

  if (interface != "org.bluez.Adapter1") {
    return;
  }

  if (changedProperties.contains("Powered")) {
    bool powered = changedProperties["Powered"].toBool();
    if (m_powered != powered) {
      m_powered = powered;
      emit poweredChanged();
    }
  }

  if (changedProperties.contains("Discoverable")) {
    bool discoverable = changedProperties["Discoverable"].toBool();
    if (m_discoverable != discoverable) {
      m_discoverable = discoverable;
      emit discoverableChanged();
    }
  }

  if (changedProperties.contains("Name")) {
    QString name = changedProperties["Name"].toString();
    if (m_adapterName != name) {
      m_adapterName = name;
      emit adapterNameChanged();
    }
  }
}
