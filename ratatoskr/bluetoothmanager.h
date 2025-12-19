/*==========================================================
 * Program : bluetoothmanager.h          Project : ratatoskr
 * Author  : Philippe Andersson + Copilot CLI.
 * Date    : 2025-12-19
 * Version : 0.01
 * Notice  : (c) Les Ateliers du Heron, 2025
 * License : GNU GPL v3 or later
 * Comment : Central Bluetooth management service using Qt D-Bus.
 * Modification History:
 * - 2025-12-19 (0.01) : Initial release.
 *========================================================*/

#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QString>
#include <QVariant>

class BluetoothManager : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool powered READ powered WRITE setPowered NOTIFY poweredChanged)
  Q_PROPERTY(bool discoverable READ discoverable WRITE setDiscoverable NOTIFY discoverableChanged)
  Q_PROPERTY(QString adapterAddress READ adapterAddress NOTIFY adapterAddressChanged)
  Q_PROPERTY(QString adapterName READ adapterName NOTIFY adapterNameChanged)

public:
  explicit BluetoothManager(QObject *parent = nullptr);
  ~BluetoothManager();

  bool powered() const { return m_powered; }
  void setPowered(bool powered);

  bool discoverable() const { return m_discoverable; }
  void setDiscoverable(bool discoverable);

  QString adapterAddress() const { return m_adapterAddress; }
  QString adapterName() const { return m_adapterName; }

  Q_INVOKABLE bool initialize();
  Q_INVOKABLE void startDiscovery();
  Q_INVOKABLE void stopDiscovery();

signals:
  void poweredChanged();
  void discoverableChanged();
  void adapterAddressChanged();
  void adapterNameChanged();
  void errorOccurred(const QString &error);

private slots:
  void onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
  bool loadAdapterProperties();
  QVariant getAdapterProperty(const QString &property);
  void setAdapterProperty(const QString &property, const QVariant &value);

  QDBusInterface *m_bluezManager;
  QDBusInterface *m_adapter;
  QString m_adapterPath;

  bool m_powered;
  bool m_discoverable;
  QString m_adapterAddress;
  QString m_adapterName;
};

#endif // BLUETOOTHMANAGER_H
