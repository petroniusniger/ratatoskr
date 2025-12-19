/*==========================================================
 * Program : devicediscovery.cpp         Project : ratatoskr
 * Author  : Philippe Andersson + Copilot CLI.
 * Date    : 2025-12-19
 * Version : 0.01
 * Notice  : (c) Les Ateliers du Heron, 2025
 * License : GNU GPL v3 or later
 * Comment : Bluetooth device discovery service using Qt D-Bus.
 * Modification History:
 * - 2025-12-19 (0.01) : Initial release.
 *========================================================*/

#include "devicediscovery.h"
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusReply>
#include <QDebug>

// BluetoothDevice implementation

BluetoothDevice::BluetoothDevice(const QString &path, QObject *parent)
  : QObject(parent)
  , m_devicePath(path)
  , m_device(nullptr)
  , m_paired(false)
  , m_connected(false)
  , m_trusted(false)
  , m_rssi(0)
{
  QDBusConnection bus = QDBusConnection::systemBus();
  m_device = new QDBusInterface(
    "org.bluez",
    m_devicePath,
    "org.bluez.Device1",
    bus,
    this
  );

  bus.connect(
    "org.bluez",
    m_devicePath,
    "org.freedesktop.DBus.Properties",
    "PropertiesChanged",
    this,
    SLOT(onPropertiesChanged(QString,QVariantMap,QStringList))
  );

  loadProperties();
}

void BluetoothDevice::loadProperties()
{
  QVariant address = getProperty("Address");
  if (address.isValid()) {
    m_address = address.toString();
  }

  QVariant name = getProperty("Name");
  if (name.isValid()) {
    m_name = name.toString();
  }

  QVariant alias = getProperty("Alias");
  if (alias.isValid()) {
    m_alias = alias.toString();
  }

  QVariant paired = getProperty("Paired");
  if (paired.isValid()) {
    m_paired = paired.toBool();
  }

  QVariant connected = getProperty("Connected");
  if (connected.isValid()) {
    m_connected = connected.toBool();
  }

  QVariant trusted = getProperty("Trusted");
  if (trusted.isValid()) {
    m_trusted = trusted.toBool();
  }

  QVariant rssi = getProperty("RSSI");
  if (rssi.isValid()) {
    m_rssi = rssi.toInt();
  }
}

QVariant BluetoothDevice::getProperty(const QString &property)
{
  if (!m_device) {
    return QVariant();
  }

  QDBusInterface props(
    "org.bluez",
    m_devicePath,
    "org.freedesktop.DBus.Properties",
    QDBusConnection::systemBus()
  );

  QDBusReply<QVariant> reply = props.call("Get", "org.bluez.Device1", property);
  if (!reply.isValid()) {
    return QVariant();
  }

  return reply.value();
}

void BluetoothDevice::setProperty(const QString &property, const QVariant &value)
{
  if (!m_device) {
    return;
  }

  QDBusInterface props(
    "org.bluez",
    m_devicePath,
    "org.freedesktop.DBus.Properties",
    QDBusConnection::systemBus()
  );

  props.call("Set", "org.bluez.Device1", property, QVariant::fromValue(QDBusVariant(value)));
}

void BluetoothDevice::updateProperties(const QVariantMap &properties)
{
  if (properties.contains("Name")) {
    QString name = properties["Name"].toString();
    if (m_name != name) {
      m_name = name;
      emit nameChanged();
    }
  }

  if (properties.contains("Alias")) {
    QString alias = properties["Alias"].toString();
    if (m_alias != alias) {
      m_alias = alias;
      emit aliasChanged();
    }
  }

  if (properties.contains("Paired")) {
    bool paired = properties["Paired"].toBool();
    if (m_paired != paired) {
      m_paired = paired;
      emit pairedChanged();
    }
  }

  if (properties.contains("Connected")) {
    bool connected = properties["Connected"].toBool();
    if (m_connected != connected) {
      m_connected = connected;
      emit connectedChanged();
    }
  }

  if (properties.contains("Trusted")) {
    bool trusted = properties["Trusted"].toBool();
    if (m_trusted != trusted) {
      m_trusted = trusted;
      emit trustedChanged();
    }
  }

  if (properties.contains("RSSI")) {
    int rssi = properties["RSSI"].toInt();
    if (m_rssi != rssi) {
      m_rssi = rssi;
      emit rssiChanged();
    }
  }
}

void BluetoothDevice::pair()
{
  if (!m_device) {
    return;
  }

  m_device->call("Pair");
}

void BluetoothDevice::trust()
{
  setProperty("Trusted", true);
}

void BluetoothDevice::connect()
{
  if (!m_device) {
    return;
  }

  m_device->call("Connect");
}

void BluetoothDevice::disconnect()
{
  if (!m_device) {
    return;
  }

  m_device->call("Disconnect");
}

void BluetoothDevice::onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
  Q_UNUSED(invalidatedProperties);

  if (interface != "org.bluez.Device1") {
    return;
  }

  updateProperties(changedProperties);
}

// DeviceDiscovery implementation

DeviceDiscovery::DeviceDiscovery(QObject *parent)
  : QAbstractListModel(parent)
  , m_bluezManager(nullptr)
  , m_discovering(false)
{
  QDBusConnection bus = QDBusConnection::systemBus();
  if (!bus.isConnected()) {
    emit errorOccurred("Cannot connect to system D-Bus");
    return;
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
    return;
  }

  QDBusMessage reply = m_bluezManager->call("GetManagedObjects");
  if (reply.type() == QDBusMessage::ErrorMessage) {
    emit errorOccurred("Cannot get managed objects: " + reply.errorMessage());
    return;
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
    return;
  }

  setupDBusMonitoring();
  loadExistingDevices();
}

DeviceDiscovery::~DeviceDiscovery()
{
  qDeleteAll(m_devices);
  delete m_bluezManager;
}

void DeviceDiscovery::setupDBusMonitoring()
{
  QDBusConnection bus = QDBusConnection::systemBus();
  
  bus.connect(
    "org.bluez",
    "/",
    "org.freedesktop.DBus.ObjectManager",
    "InterfacesAdded",
    this,
    SLOT(onInterfacesAdded(QDBusObjectPath,QVariantMap))
  );

  bus.connect(
    "org.bluez",
    "/",
    "org.freedesktop.DBus.ObjectManager",
    "InterfacesRemoved",
    this,
    SLOT(onInterfacesRemoved(QDBusObjectPath,QStringList))
  );
}

void DeviceDiscovery::loadExistingDevices()
{
  if (!m_bluezManager) {
    return;
  }

  QDBusMessage reply = m_bluezManager->call("GetManagedObjects");
  if (reply.type() == QDBusMessage::ErrorMessage) {
    return;
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

    if (interfaces.contains("org.bluez.Device1") && path.startsWith(m_adapterPath + "/")) {
      QVariantMap deviceProps = interfaces["org.bluez.Device1"].toMap();
      addDevice(path, deviceProps);
    }
  }
  arg.endMap();
}

void DeviceDiscovery::addDevice(const QString &path, const QVariantMap &properties)
{
  for (BluetoothDevice *dev : m_devices) {
    if (dev->path() == path) {
      dev->updateProperties(properties);
      return;
    }
  }

  beginInsertRows(QModelIndex(), m_devices.count(), m_devices.count());
  BluetoothDevice *device = new BluetoothDevice(path, this);
  if (!properties.isEmpty()) {
    device->updateProperties(properties);
  }
  m_devices.append(device);
  endInsertRows();

  emit countChanged();
  emit deviceFound(device);
}

void DeviceDiscovery::removeDevice(const QString &path)
{
  for (int i = 0; i < m_devices.count(); ++i) {
    if (m_devices[i]->path() == path) {
      beginRemoveRows(QModelIndex(), i, i);
      delete m_devices.takeAt(i);
      endRemoveRows();
      emit countChanged();
      return;
    }
  }
}

void DeviceDiscovery::startDiscovery()
{
  if (m_adapterPath.isEmpty()) {
    emit errorOccurred("No adapter available");
    return;
  }

  QDBusInterface adapter(
    "org.bluez",
    m_adapterPath,
    "org.bluez.Adapter1",
    QDBusConnection::systemBus()
  );

  QDBusMessage reply = adapter.call("StartDiscovery");
  if (reply.type() == QDBusMessage::ErrorMessage) {
    emit errorOccurred("Failed to start discovery: " + reply.errorMessage());
    return;
  }

  m_discovering = true;
  emit discoveringChanged();
}

void DeviceDiscovery::stopDiscovery()
{
  if (m_adapterPath.isEmpty()) {
    return;
  }

  QDBusInterface adapter(
    "org.bluez",
    m_adapterPath,
    "org.bluez.Adapter1",
    QDBusConnection::systemBus()
  );

  adapter.call("StopDiscovery");
  m_discovering = false;
  emit discoveringChanged();
}

BluetoothDevice* DeviceDiscovery::getDevice(int index)
{
  if (index < 0 || index >= m_devices.count()) {
    return nullptr;
  }
  return m_devices.at(index);
}

int DeviceDiscovery::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return m_devices.count();
}

QVariant DeviceDiscovery::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || index.row() >= m_devices.count()) {
    return QVariant();
  }

  BluetoothDevice *device = m_devices.at(index.row());

  switch (role) {
    case AddressRole:
      return device->address();
    case NameRole:
      return device->name();
    case AliasRole:
      return device->alias();
    case PairedRole:
      return device->paired();
    case ConnectedRole:
      return device->connected();
    case TrustedRole:
      return device->trusted();
    case RssiRole:
      return device->rssi();
    case DeviceObjectRole:
      return QVariant::fromValue(device);
    default:
      return QVariant();
  }
}

QHash<int, QByteArray> DeviceDiscovery::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[AddressRole] = "address";
  roles[NameRole] = "deviceName";
  roles[AliasRole] = "alias";
  roles[PairedRole] = "paired";
  roles[ConnectedRole] = "connected";
  roles[TrustedRole] = "trusted";
  roles[RssiRole] = "rssi";
  roles[DeviceObjectRole] = "deviceObject";
  return roles;
}

void DeviceDiscovery::onInterfacesAdded(const QDBusObjectPath &objectPath, const QVariantMap &interfaces)
{
  QString path = objectPath.path();
  
  if (interfaces.contains("org.bluez.Device1") && path.startsWith(m_adapterPath + "/")) {
    QVariantMap deviceProps = interfaces["org.bluez.Device1"].toMap();
    addDevice(path, deviceProps);
  }
}

void DeviceDiscovery::onInterfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces)
{
  QString path = objectPath.path();
  
  if (interfaces.contains("org.bluez.Device1")) {
    removeDevice(path);
  }
}
