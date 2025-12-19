/*==========================================================
 * Program : obextransfer.h              Project : ratatoskr
 * Author  : Philippe Andersson + Copilot CLI.
 * Date    : 2025-12-19
 * Version : 0.01
 * Notice  : (c) Les Ateliers du Heron, 2025
 * License : GNU GPL v3 or later
 * Comment : OBEX outgoing file transfer service using D-Bus.
 * Modification History:
 * - 2025-12-19 (0.01) : Initial release.
 *========================================================*/

#ifndef OBEXTRANSFER_H
#define OBEXTRANSFER_H

#include <QObject>
#include <QString>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QVariant>

class OBEXTransfer : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString filename READ filename NOTIFY filenameChanged)
  Q_PROPERTY(qint64 size READ size NOTIFY sizeChanged)
  Q_PROPERTY(qint64 transferred READ transferred NOTIFY transferredChanged)
  Q_PROPERTY(Status status READ status NOTIFY statusChanged)
  Q_PROPERTY(int progress READ progress NOTIFY progressChanged)

public:
  enum Status {
    StatusQueued,
    StatusActive,
    StatusComplete,
    StatusError
  };
  Q_ENUM(Status)

  explicit OBEXTransfer(const QString &sessionPath, const QString &transferPath, QObject *parent = nullptr);
  ~OBEXTransfer();

  QString filename() const { return m_filename; }
  qint64 size() const { return m_size; }
  qint64 transferred() const { return m_transferred; }
  Status status() const { return m_status; }
  int progress() const;

  QString transferPath() const { return m_transferPath; }

  Q_INVOKABLE void cancel();

signals:
  void filenameChanged();
  void sizeChanged();
  void transferredChanged();
  void statusChanged();
  void progressChanged();
  void completed();
  void failed(const QString &error);

private slots:
  void onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
  void loadProperties();
  QVariant getProperty(const QString &property);
  Status statusFromString(const QString &statusStr);

  QString m_sessionPath;
  QString m_transferPath;
  QDBusInterface *m_transfer;

  QString m_filename;
  qint64 m_size;
  qint64 m_transferred;
  Status m_status;
};

class OBEXTransferService : public QObject
{
  Q_OBJECT

public:
  explicit OBEXTransferService(QObject *parent = nullptr);
  ~OBEXTransferService();

  Q_INVOKABLE OBEXTransfer* sendFile(const QString &deviceAddress, const QString &filePath);
  Q_INVOKABLE void cancelTransfer(OBEXTransfer *transfer);

signals:
  void transferStarted(OBEXTransfer *transfer);
  void errorOccurred(const QString &error);

private:
  QString createSession(const QString &deviceAddress);
  void removeSession(const QString &sessionPath);
  
  QDBusInterface *m_obexClient;
  QList<OBEXTransfer*> m_activeTransfers;
};

#endif // OBEXTRANSFER_H
