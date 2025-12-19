/*==========================================================
 * Program : devicediscovery.h           Project : ratatoskr
 * Author  : Philippe Andersson + Copilot CLI.
 * Date    : 2025-12-19
 * Version : 0.01
 * Notice  : (c) Les Ateliers du Heron, 2025
 * License : GNU GPL v3 or later
 * Comment : Bluetooth device discovery service using Qt D-Bus.
 * Modification History:
 * - 2025-12-19 (0.01) : Initial release.
 *========================================================*/

#ifndef DEVICEDISCOVERY_H
#define DEVICEDISCOVERY_H

#include <QObject>
#include <QList>
#include <QAbstractListModel>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QString>
#include <QVariant>

class BluetoothDevice : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString address READ address CONSTANT)
  Q_PROPERTY(QString name READ name NOTIFY nameChanged)
  Q_PROPERTY(QString alias READ alias NOTIFY aliasChanged)
  Q_PROPERTY(bool paired READ paired NOTIFY pairedChanged)
  Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
  Q_PROPERTY(bool trusted READ trusted NOTIFY trustedChanged)
  Q_PROPERTY(int rssi READ rssi NOTIFY rssiChanged)

public:
  explicit BluetoothDevice(const QString &path, QObject *parent = nullptr);

  QString address() const { return m_address; }
  QString name() const { return m_name; }
  QString alias() const { return m_alias; }
  bool paired() const { return m_paired; }
  bool connected() const { return m_connected; }
  bool trusted() const { return m_trusted; }
  int rssi() const { return m_rssi; }
  QString path() const { return m_devicePath; }

  Q_INVOKABLE void pair();
  Q_INVOKABLE void trust();
  Q_INVOKABLE void connect();
  Q_INVOKABLE void disconnect();

  void updateProperties(const QVariantMap &properties);

signals:
  void nameChanged();
  void aliasChanged();
  void pairedChanged();
  void connectedChanged();
  void trustedChanged();
  void rssiChanged();

private slots:
  void onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
  void loadProperties();
  QVariant getProperty(const QString &property);
  void setProperty(const QString &property, const QVariant &value);

  QString m_devicePath;
  QDBusInterface *m_device;

  QString m_address;
  QString m_name;
  QString m_alias;
  bool m_paired;
  bool m_connected;
  bool m_trusted;
  int m_rssi;
};

class DeviceDiscovery : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY(bool discovering READ discovering NOTIFY discoveringChanged)
  Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
  enum DeviceRoles {
    AddressRole = Qt::UserRole + 1,
    NameRole,
    AliasRole,
    PairedRole,
    ConnectedRole,
    TrustedRole,
    RssiRole,
    DeviceObjectRole
  };

  explicit DeviceDiscovery(QObject *parent = nullptr);
  ~DeviceDiscovery();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  bool discovering() const { return m_discovering; }
  int count() const { return m_devices.count(); }

  Q_INVOKABLE void startDiscovery();
  Q_INVOKABLE void stopDiscovery();
  Q_INVOKABLE BluetoothDevice* getDevice(int index);

signals:
  void discoveringChanged();
  void countChanged();
  void deviceFound(BluetoothDevice *device);
  void errorOccurred(const QString &error);

private slots:
  void onInterfacesAdded(const QDBusObjectPath &objectPath, const QVariantMap &interfaces);
  void onInterfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces);

private:
  void setupDBusMonitoring();
  void loadExistingDevices();
  void addDevice(const QString &path, const QVariantMap &properties = QVariantMap());
  void removeDevice(const QString &path);

  QDBusInterface *m_bluezManager;
  QString m_adapterPath;
  QList<BluetoothDevice*> m_devices;
  bool m_discovering;
};

#endif // DEVICEDISCOVERY_H
