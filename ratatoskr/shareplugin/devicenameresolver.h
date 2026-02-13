/*==========================================================
 * Program : devicenameresolver.h        Project : ratatoskr
 * Author  : Philippe Andersson + Copilot CLI.
 * Date    : 2026-02-05
 * Version : 0.0.3
 * Notice  : (c) Les Ateliers du Heron, 2025-2026
 * License : GNU GPL v3 or later
 * Comment : Resolves Bluetooth device MAC addresses to names.
 * Modification History:
 * - 2026-02-02 (0.0.1) : Initial release.
 * - 2026-02-02 (0.0.2) : Added D-Bus monitoring for dynamic name updates.
 * - 2026-02-05 (0.0.3) : Added MAC-based name detection.
 *========================================================*/

#ifndef DEVICENAMERESOLVER_H
#define DEVICENAMERESOLVER_H

#include <QObject>
#include <QString>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QVariant>
#include <QMap>
#include <QStringList>

class DeviceNameResolver : public QObject
{
  Q_OBJECT

public:
  explicit DeviceNameResolver(QObject *parent = nullptr);
  ~DeviceNameResolver();

  Q_INVOKABLE QString resolveDeviceName(const QString &address);
  Q_INVOKABLE void monitorDevice(const QString &address);
  Q_INVOKABLE bool isMacBasedName(const QString &name) const;

signals:
  void deviceNameChanged(const QString &address, const QString &name);

private slots:
  void onInterfacesAdded(const QDBusObjectPath &objectPath, const QVariantMap &interfaces);
  void onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
  QString getDeviceProperty(const QString &devicePath, const QString &property);
  QString addressToPath(const QString &address);
  QString pathToAddress(const QString &devicePath);
  QString findAdapterPath();
  void setupDBusMonitoring();

  QString m_adapterPath;
  QMap<QString, QString> m_monitoredDevices;
};

#endif // DEVICENAMERESOLVER_H
