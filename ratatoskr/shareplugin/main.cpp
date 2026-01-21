/*==========================================================
 * Program : main.cpp              Project : ratatoskr
 * Author  : Michael Zanetti, Ian L., Philippe Andersson
 * Date    : 2026-01-21
 * Version : 0.0.4
 * Notice  : (c) Original work by Michael Zanetti, Canonical
 *           Adapted by Ian L. and Philippe Andersson
 * License : GNU GPL v3 or later
 * Comment : ContentHub shareplugin entry point
 * Modification History:
 * - 2025-12-18 (0.0.1) : Adapted from ubtd-20.04.
 * - 2025-12-25 (0.0.3) : Switched to QQmlApplicationEngine for better lifecycle.
 * - 2026-01-21 (0.0.4) : Fixed applicationName to match manifest.
 *========================================================*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "adapter.h"
#include "bttransfer.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<BtTransfer>("Shareplugin", 0, 1, "BtTransfer");

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::quit, 
                     &app, &QCoreApplication::quit);

    engine.load(QUrl(QStringLiteral("qrc:///Main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
