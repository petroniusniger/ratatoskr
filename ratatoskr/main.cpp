/*==========================================================
 * Program : main.cpp                    Project : ratatoskr
 * Author  : Philippe Andersson + Copilot CLI.
 * Date    : 2025-12-19
 * Version : 0.0.1
 * Notice  : (c) Les Ateliers du Heron, 2025
 * License : GNU GPL v3 or later
 * Comment : Main application entry point.
 *           Based on ubtd by Michael Zanetti and Ian L.
 * Modification History:
 * - 2025-12-18 (0.0.1) : Initial adaptation from ubtd-20.04.
 * - 2025-12-19 (0.0.1) : Added BluetoothManager, DeviceDiscovery, OBEXTransfer.
 *========================================================*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>

#include "adapter.h"
#include "obexd.h"
#include "bluetoothmanager.h"
#include "devicediscovery.h"
#include "obextransfer.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("ratatoskr.philipa");

    Adapter adapter;
    Obexd obexd;
    BluetoothManager bluetoothManager;
    DeviceDiscovery deviceDiscovery;
    OBEXTransferService obexTransferService;

    QQuickView view;

    view.rootContext()->setContextProperty("obexd", &obexd);
    view.rootContext()->setContextProperty("bluetoothManager", &bluetoothManager);
    view.rootContext()->setContextProperty("deviceDiscovery", &deviceDiscovery);
    view.rootContext()->setContextProperty("obexTransferService", &obexTransferService);
    qmlRegisterUncreatableType<Transfer>("Ratatoskr", 1, 0, "Transfer", "get them from obexd");
    qmlRegisterUncreatableType<BluetoothDevice>("Ratatoskr", 1, 0, "BluetoothDevice", "get them from deviceDiscovery");
    qmlRegisterUncreatableType<OBEXTransfer>("Ratatoskr", 1, 0, "OBEXTransfer", "get them from obexTransferService");

    bluetoothManager.initialize();

    view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    return app.exec();
}

