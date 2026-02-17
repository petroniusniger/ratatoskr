/*==========================================================
 * Program : Main.qml                    Project : ratatoskr
 * Author  : Michael Zanetti, Ian L., Philippe Andersson
 * Date    : 2026-02-17
 * Version : 0.1.6
 * Notice  : (c) Original work by Michael Zanetti, Canonical
 *           Adapted by Ian L. and Philippe Andersson
 * License : GNU GPL v3 or later
 * Comment : ContentHub shareplugin QML interface
 * Modification History:
 * - 2025-12-18 (0.0.1) : Adapted from ubtd-20.04.
 * - 2025-12-25 (0.0.3) : Switched to QQmlApplicationEngine.
 * - 2026-01-21 (0.0.4) : Fixed applicationName to match manifest.
 * - 2026-01-22 (0.0.5) : Fixed ContentHub integration (renamed transfer vars).
 * - 2026-01-29 (0.0.17): Added Window root element required by QQmlApplicationEngine.
 * - 2026-01-29 (0.1.0) : Removed file preview, cleaned debug logs, fixed indentation.
 * - 2026-02-02 (0.1.1) : Added device name resolution using DeviceNameResolver.
 * - 2026-02-02 (0.1.2) : Added dynamic name update monitoring for newly discovered devices.
 * - 2026-02-05 (0.1.3) : Fixed list scrollability and added MAC-based name filtering.
 * - 2026-02-12 (0.1.4) : Added warning dialog for direct launch from App Drawer.
 * - 2026-02-12 (0.1.5) : Added comments explaining timing and Bluetooth state management.
 * - 2026-02-17 (0.1.6) : Changed app author name to match GitHub account.
 *========================================================*/

import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import Lomiri.Components 1.3
import Lomiri.Components.Popups 1.3
import QtBluetooth 5.4
import Shareplugin 0.1
import Lomiri.Content 1.3

Window {
    id: window
    visible: true
    width: units.gu(50)
    height: units.gu(75)

    MainView {
        id: root
        anchors.fill: parent
        applicationName: "ratatoskr.petroniusniger"
        anchorToKeyboard: true

        property var fileNames: []
        property bool peerSelected: false

        Timer {
            id: startupCheckTimer
            interval: 500
            running: true
            repeat: false
            onTriggered: {
                console.log("Startup check triggered, fileNames.length:", root.fileNames.length)
                if (root.fileNames.length === 0) {
                    console.log("SharePlugin launched directly without files - showing warning")
                    btModel.running = false
                    btModel.continuousDiscovery = false
                    PopupUtils.open(directLaunchWarningDialog, root)
                }
            }
        }

        Component {
            id: directLaunchWarningDialog
            Dialog {
                id: dialog
                title: i18n.tr("Wrong Launch Method")
                text: i18n.tr("This plugin should not be started directly from the App Drawer.\n\nPlease use the Share button when viewing a file to send it via Bluetooth.")

                Button {
                    text: i18n.tr("Close")
                    color: LomiriColors.orange
                    onClicked: {
                        PopupUtils.close(dialog)
                        Qt.quit()
                    }
                }
            }
        }

        ListModel {
            id: deviceListModel
        }

        BtTransfer {
            id: btTransfer
        }

        DeviceNameResolver {
            id: deviceNameResolver
            onDeviceNameChanged: {
                console.log("Device name updated:", address, "->", name);
                for (var i = 0; i < deviceListModel.count; i++) {
                    if (deviceListModel.get(i).remoteAddress === address) {
                        deviceListModel.setProperty(i, "deviceName", name);
                        deviceListModel.setProperty(i, "name", name);
                        break;
                    }
                }
            }
        }

        Connections {
            target: ContentHub

            onShareRequested: {
                console.log("ContentHub share requested, transfer:", transfer)
                startupCheckTimer.stop()
                var tmp = []
                for (var i = 0; i < transfer.items.length; i++) {
                    var filePath = String(transfer.items[i].url).replace('file://', '')
                    console.log("Should share file", filePath)
                    tmp.push(filePath);
                }
                root.fileNames = tmp
            }
        }

        Connections {
            target: Qt.application
            onAboutToQuit: {
                btModel.running = false
                btModel.continuousDiscovery = false
            }
        }

        // Bluetooth device discovery with continuous scanning
        // continuousDiscovery flag prevents normal Bluetooth stop/start cycles
        BluetoothDiscoveryModel {
            id: btModel
            // Custom property to enable auto-restart after discovery completes
            property bool continuousDiscovery: true
            onContinuousDiscoveryChanged: {
                if (continuousDiscovery && !running) {
                    running = true;
                } else if (!continuousDiscovery && running) {
                    running = false;
                }
            }

            running: false

            Component.onCompleted: {
                // Qt.callLater avoids signal recursion during initialization
                Qt.callLater(function() {
                    running = true
                })
            }

            onRunningChanged: {
                // Auto-restart discovery when it stops (unless disabled by continuousDiscovery flag)
                if (!running && continuousDiscovery) {
                    Qt.callLater(function() {
                        running = true
                    })
                }
            }

            discoveryMode: BluetoothDiscoveryModel.DeviceDiscovery
            onServiceDiscovered: console.log("Found new service " + service.deviceAddress + " " + service.deviceName + " " + service.serviceName);
            onDeviceDiscovered: {
                console.log("New device: " + device)
                // Deduplication: update existing device instead of adding duplicate
                for (var i = 0; i < deviceListModel.count; i++) {
                    if (deviceListModel.get(i).remoteAddress === device) {
                        return;
                    }
                }
                var deviceName = deviceNameResolver.resolveDeviceName(device);
                console.log("Resolved device name:", deviceName, "for address:", device);
                
                // Filter out devices with MAC address as name (e.g., "00-11-22-33-44-55")
                // These are typically devices without proper Bluetooth name set
                if (deviceNameResolver.isMacBasedName(deviceName)) {
                    console.log("Skipping device with MAC-based name:", deviceName);
                    return;
                }
                
                deviceListModel.append({
                    "remoteAddress": device,
                    "deviceName": deviceName,
                    "name": deviceName
                });
                // Start monitoring for name changes (some devices broadcast name after discovery)
                deviceNameResolver.monitorDevice(device);
            }
            onErrorChanged: {
                switch (btModel.error) {
                case BluetoothDiscoveryModel.PoweredOffError:
                    console.log("Error: Bluetooth device not turned on"); break;
                case BluetoothDiscoveryModel.InputOutputError:
                    console.log("Error: Bluetooth I/O Error"); break;
                case BluetoothDiscoveryModel.InvalidBluetoothAdapterError:
                    console.log("Error: Invalid Bluetooth Adapter Error"); break;
                case BluetoothDiscoveryModel.NoError:
                    break;
                default:
                    console.log("Error: Unknown Error"); break;
                }
            }
        }

        Page {
            id: page
            anchors.fill: parent
            header: PageHeader {
                title: i18n.tr("Share via Bluetooth")
                leadingActionBar.actions: [
                    Action {
                        iconName: "close"
                        onTriggered: Qt.quit()
                    }

                ]
            }

            Rectangle {
                id: progressBar
                anchors {
                    left: parent.left; top: parent.top; right: parent.right; topMargin: page.header.height
                }
                height: units.dp(5)
                visible: btModel.running
                Rectangle {
                    id: progressElement
                    width: parent.width / 3
                    height: parent.height
                    color: LomiriColors.blue
                }
                property int targetX: page.width - progressElement.width
                onTargetXChanged: {
                    if (visible) {
                        inquiryAnimation.restart()
                    }
                }
                SequentialAnimation {
                    id: inquiryAnimation
                    loops: Animation.Infinite
                    running: progressBar.visible
                    NumberAnimation {
                        target: progressElement
                        duration: 2000
                        property: "x"
                        from: 0
                        to: progressBar.targetX
                    }
                    NumberAnimation {
                        target: progressElement
                        property: "x"
                        duration: 2000
                        from: progressBar.targetX
                        to: 0
                    }
                }
            }

            Item {
                anchors.fill: parent
                anchors.topMargin: page.header.height + progressBar.height

                Connections {
                    target: deviceListModel
                    onCountChanged: {
                        if (deviceListView.model === deviceListModel) {
                            deviceListView.forceLayout()
                        }
                    }
                }

                ListView {
                    id: deviceListView
                    anchors.fill: parent
                    model: deviceListModel
                    visible: !root.peerSelected
                    clip: true
                    spacing: units.gu(1)
                    interactive: true
                    boundsBehavior: Flickable.DragAndOvershootBounds

                        onModelChanged: {
                            forceLayout()
                        }

                    delegate: ListItem {
                        width: ListView.view ? ListView.view.width : parent.width
                        height: units.gu(7)
                        
                        ListItemLayout {
                            title.text: (model.deviceName ? model.deviceName : model.name) || model.remoteAddress
                        }

                        onClicked: {
                            btModel.continuousDiscovery = false;
                            for (var i = 0; i < root.fileNames.length; i++) {
                                btTransfer.sendFile(model.remoteAddress, root.fileNames[i])
                            }
                            root.peerSelected = true;
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width - units.gu(4)
                    anchors.centerIn: parent
                    spacing: units.gu(2)
                    visible: root.peerSelected
                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: btTransfer.error ? "Transfer failed."
                                : btTransfer.finished ? (root.fileNames.length == 1 ? "File transferred." : "All files transferred.")
                                : "Transferring..."
                        fontSize: "large"
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        value: btTransfer.progress
                        visible: !btTransfer.finished && !btTransfer.error
                    }

                    Button {
                        Layout.fillWidth: true
                        text: i18n.tr("Close")
                        color: btTransfer.error ? LomiriColors.red : LomiriColors.green
                        onClicked: Qt.quit();
                        visible: btTransfer.finished || btTransfer.error
                    }
                }
            }
        }
    }
}

