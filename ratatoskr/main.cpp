/*==========================================================
 * Program : main.cpp                    Project : ratatoskr
 * Author  : Philippe Andersson + Copilot CLI.
 * Date    : 2026-02-17
 * Version : 0.0.5
 * Notice  : (c) Les Ateliers du Heron, 2025
 * License : GNU GPL v3 or later
 * Comment : Main application entry point.
 *           Based on 'ubtd' by Michael Zanetti and Ian L.
 * Modification History:
 * - 2025-12-18 (0.0.1) : Initial adaptation from ubtd-20.04.
 * - 2025-12-19 (0.0.2) : Added BluetoothManager, DeviceDiscovery, OBEXTransfer.
 * - 2026-02-11 (0.0.3) : Added version logging at startup.
 * - 2026-02-12 (0.0.4) : Removed unused BluetoothManager and OBEXTransferService.
 * - 2026-02-17 (0.0.5) : Changed app author name to match GitHub account.
 *========================================================*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QDebug>

#include "adapter.h"
#include "obexd.h"
#include "devicediscovery.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("ratatoskr.petroniusniger");

    qDebug() << "Ratatoskr" << BUILD_VERSION << "starting...";

    Adapter adapter;
    Obexd obexd;
    DeviceDiscovery deviceDiscovery;

    QQuickView view;

    view.rootContext()->setContextProperty("obexd", &obexd);
    view.rootContext()->setContextProperty("deviceDiscovery", &deviceDiscovery);
    qmlRegisterUncreatableType<Transfer>("Ratatoskr", 1, 0, "Transfer", "get them from obexd");
    qmlRegisterUncreatableType<BluetoothDevice>("Ratatoskr", 1, 0, "BluetoothDevice", "get them from deviceDiscovery");

    view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    return app.exec();
}

