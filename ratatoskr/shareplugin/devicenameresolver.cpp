/*==========================================================
 * Program : devicenameresolver.cpp     Project : ratatoskr
 * Author  : Philippe Andersson + Copilot CLI.
 * Date    : 2026-02-02
 * Version : 0.02
 * Notice  : (c) Les Ateliers du Heron, 2025-2026
 * License : GNU GPL v3 or later
 * Comment : Resolves Bluetooth device MAC addresses to names.
 * Modification History:
 * - 2026-02-02 (0.01) : Initial release.
 * - 2026-02-02 (0.02) : Added D-Bus monitoring for dynamic name updates.
 *========================================================*/

#include "devicenameresolver.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QDebug>

DeviceNameResolver::DeviceNameResolver(QObject *parent)
  : QObject(parent)
{
  m_adapterPath = findAdapterPath();
  if (m_adapterPath.isEmpty()) {
    qWarning() << "DeviceNameResolver: No Bluetooth adapter found";
  } else {
    setupDBusMonitoring();
  }
}

DeviceNameResolver::~DeviceNameResolver()
{
}

QString DeviceNameResolver::findAdapterPath()
{
  QDBusInterface manager(
    "org.bluez",
    "/",
    "org.freedesktop.DBus.ObjectManager",
    QDBusConnection::systemBus()
  );

  QDBusMessage reply = manager.call("GetManagedObjects");
  if (reply.type() == QDBusMessage::ErrorMessage) {
    qWarning() << "DeviceNameResolver: Cannot get managed objects:" << reply.errorMessage();
    return QString();
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
      return path;
    }
  }
  arg.endMap();

  return QString();
}

void DeviceNameResolver::setupDBusMonitoring()
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
}

QString DeviceNameResolver::pathToAddress(const QString &devicePath)
{
  if (!devicePath.startsWith(m_adapterPath + "/dev_")) {
    return QString();
  }
  
  QString cleanPath = devicePath;
  cleanPath.remove(m_adapterPath + "/dev_");
  cleanPath.replace("_", ":");
  return cleanPath;
}

QString DeviceNameResolver::addressToPath(const QString &address)
{
  if (m_adapterPath.isEmpty()) {
    return QString();
  }

  QString cleanAddress = address;
  cleanAddress.replace(":", "_");
  return m_adapterPath + "/dev_" + cleanAddress;
}

QString DeviceNameResolver::getDeviceProperty(const QString &devicePath, const QString &property)
{
  QDBusInterface props(
    "org.bluez",
    devicePath,
    "org.freedesktop.DBus.Properties",
    QDBusConnection::systemBus()
  );

  QDBusReply<QVariant> reply = props.call("Get", "org.bluez.Device1", property);
  if (!reply.isValid()) {
    return QString();
  }

  return reply.value().toString();
}

QString DeviceNameResolver::resolveDeviceName(const QString &address)
{
  QString devicePath = addressToPath(address);
  if (devicePath.isEmpty()) {
    return address;
  }

  QString alias = getDeviceProperty(devicePath, "Alias");
  if (!alias.isEmpty()) {
    return alias;
  }

  QString name = getDeviceProperty(devicePath, "Name");
  if (!name.isEmpty()) {
    return name;
  }

  return address;
}

void DeviceNameResolver::monitorDevice(const QString &address)
{
  QString devicePath = addressToPath(address);
  if (devicePath.isEmpty()) {
    return;
  }

  m_monitoredDevices[devicePath] = address;

  QDBusConnection bus = QDBusConnection::systemBus();
  bus.connect(
    "org.bluez",
    devicePath,
    "org.freedesktop.DBus.Properties",
    "PropertiesChanged",
    this,
    SLOT(onPropertiesChanged(QString,QVariantMap,QStringList))
  );
}

void DeviceNameResolver::onInterfacesAdded(const QDBusObjectPath &objectPath, const QVariantMap &interfaces)
{
  QString path = objectPath.path();
  
  if (!interfaces.contains("org.bluez.Device1")) {
    return;
  }

  if (!m_monitoredDevices.contains(path)) {
    return;
  }

  QVariantMap deviceProps = interfaces["org.bluez.Device1"].toMap();
  QString address = m_monitoredDevices[path];
  
  QString name;
  if (deviceProps.contains("Alias")) {
    name = deviceProps["Alias"].toString();
  } else if (deviceProps.contains("Name")) {
    name = deviceProps["Name"].toString();
  }

  if (!name.isEmpty()) {
    emit deviceNameChanged(address, name);
  }
}

void DeviceNameResolver::onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
  Q_UNUSED(invalidatedProperties);
  
  if (interface != "org.bluez.Device1") {
    return;
  }

  QString devicePath = sender() ? sender()->property("__qdbus_path").toString() : QString();
  if (devicePath.isEmpty() || !m_monitoredDevices.contains(devicePath)) {
    return;
  }

  QString address = m_monitoredDevices[devicePath];
  QString name;

  if (changedProperties.contains("Alias")) {
    name = changedProperties["Alias"].toString();
  } else if (changedProperties.contains("Name")) {
    name = changedProperties["Name"].toString();
  }

  if (!name.isEmpty()) {
    emit deviceNameChanged(address, name);
  }
}
