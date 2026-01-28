/*==========================================================
 * Program : Main.qml              Project : ratatoskr
 * Author  : Michael Zanetti, Ian L., Philippe Andersson
 * Date    : 2026-01-27
 * Version : 0.0.12
 * Notice  : (c) Original work by Michael Zanetti, Canonical
 *           Adapted by Ian L. and Philippe Andersson
 * License : GNU GPL v3 or later
 * Comment : ContentHub shareplugin QML interface
 * Modification History:
 * - 2025-12-18 (0.0.1) : Adapted from ubtd-20.04.
 * - 2025-12-25 (0.0.3) : Switched to QQmlApplicationEngine.
 * - 2026-01-21 (0.0.4) : Fixed applicationName to match manifest.
 * - 2026-01-22 (0.0.5) : Fixed ContentHub integration (renamed transfer vars).
 * - 2026-01-23 (0.0.6) : Fixed device list visibility (hide file preview when empty).
 * - 2026-01-23 (0.0.7) : Fixed Page anchoring to fill parent MainView.
 * - 2026-01-23 (0.0.8) : Removed fixed MainView dimensions to allow proper sizing.
 * - 2026-01-23 (0.0.9) : Added debug logging to ListView to diagnose display issue.
 * - 2026-01-27 (0.0.10): Fixed ListView delegate rendering (removed wrapper Rectangle, added explicit height).
 * - 2026-01-27 (0.0.11): Added extensive debug logging to track model-view binding.
 * - 2026-01-27 (0.0.12): Added forceLayout() and Connections to fix ListView update issue.
 *========================================================*/

import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import Lomiri.Components 1.3
import QtBluetooth 5.4
import Shareplugin 0.1
import Lomiri.Content 1.3

MainView {
    id: root
    applicationName: "ratatoskr.philipa"
    anchorToKeyboard: true

    property var fileNames: []
    property bool peerSelected: false

    ListModel {
        id: deviceListModel
        
        onCountChanged: {
            console.log("deviceListModel count changed to:", count)
        }
    }

    BtTransfer {
        id: btTransfer
    }

    Connections {
        target: ContentHub

        onShareRequested: {
            console.log("ContentHub share requested, transfer:", transfer)
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
            console.log("SharePlugin: Cleaning up before quit")
            btModel.running = false
            btModel.continuousDiscovery = false
        }
    }

    BluetoothDiscoveryModel {
        id: btModel
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
            Qt.callLater(function() {
                running = true
            })
        }

        onRunningChanged: {
            if (!running && continuousDiscovery) {
                Qt.callLater(function() {
                    running = true
                })
            }
        }

        discoveryMode: BluetoothDiscoveryModel.DeviceDiscovery
        onDiscoveryModeChanged: console.log("Discovery mode: " + discoveryMode)
        onServiceDiscovered: console.log("Found new service " + service.deviceAddress + " " + service.deviceName + " " + service.serviceName);
        onDeviceDiscovered: {
            console.log("New device: " + device)
            for (var i = 0; i < deviceListModel.count; i++) {
                if (deviceListModel.get(i).remoteAddress === device) {
                    return;
                }
            }
            deviceListModel.append({
                "remoteAddress": device,
                "deviceName": device,
                "name": device
            });
            console.log("Model count after append:", deviceListModel.count)
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

        GridLayout {
            anchors.fill: parent
            anchors.topMargin: page.header.height + progressBar.height
            columns: width > height ? 2 : 1

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: root.fileNames.length > 0
                GridLayout {
                    anchors.fill: parent
                    columns: Math.ceil(Math.sqrt(root.fileNames.length))

                    Repeater {
                        model: root.fileNames
                        Item {
                            Layout.fillHeight: true
                            Layout.fillWidth: true

                            LomiriShape {
                                anchors.fill: parent
                                anchors.margins: units.gu(2)
                                aspect: LomiriShape.DropShadow
                                sourceFillMode: Image.PreserveAspectCrop
                                source: Image {
                                    id: transferredImage
                                    source: "file://" + modelData
                                }
                            }

                            Icon {
                                anchors.fill: parent
                                anchors.margins: units.gu(2)
                                visible: transferredImage.status === Image.Error
                                name: {
                                    var extension = modelData.split(".").pop()
                                    switch(extension) {
                                    case "pdf":
                                        return "application-pdf-symbolic";
                                    case "tar":
                                    case "gz":
                                    case "gzip":
                                    case "zip":
                                    case "xz":
                                        return "application-x-archive-symbolic";
                                    case "mp3":
                                    case "ogg":
                                    case "wav":
                                    case "flac":
                                        return "audio-x-generic-symbolic";
                                    case "jpg":
                                    case "gif":
                                    case "jpeg":
                                    case "png":
                                    case "webp":
                                        return "image-x-generic-symbolic";
                                    case "click":
                                    case "deb":
                                        return "package-x-generic-symbolic";
                                    case "txt":
                                        return "text-generic-symbolic";
                                    case "mp4":
                                    case "mkv":
                                    case "avi":
                                    case "mpeg":
                                    case "mpg":
                                        return "video-x-generic-symbolic";
                                    default:
                                        return "empty-symbolic";
                                    }
                                }
                            }

                            Rectangle {
                                anchors.centerIn: parent
                                color: "white"
                                border.width: units.dp(2)
                                border.color: LomiriColors.blue
                                radius: width / 2
                                height: units.gu(8)
                                width: height
                                visible: isInProgress || isDone || isError

                                property bool isInProgress: btTransfer.currentFile === modelData
                                property bool isDone: false
                                property bool isError: false
                                onIsInProgressChanged: {
                                    print("isinprogresschanged:", isInProgress, btTransfer.error)
                                    if (!isInProgress) {
                                        isDone = !btTransfer.error;
                                        isError = !!btTransfer.error
                                    }
                                }

                                ActivityIndicator {
                                    anchors.centerIn: parent
                                    visible: running
                                    running: !btTransfer.finished && btTransfer.progress > 0 && !parent.isDone
                                }

                                Icon {
                                    anchors.centerIn: parent
                                    width: units.gu(6)
                                    height: width
                                    name: parent.isDone ? "tick" : "close"
                                    color: parent.isDone ? LomiriColors.green : LomiriColors.red
                                    visible: parent.isDone || parent.isError
                                }
                            }

                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true

                Connections {
                    target: deviceListModel
                    onCountChanged: {
                        console.log("Connections: deviceListModel count is now:", deviceListModel.count)
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
                    cacheBuffer: 0

                    Component.onCompleted: {
                        console.log("ListView created, model count:", deviceListModel.count)
                        console.log("ListView model is:", model)
                    }
                    
                    onCountChanged: {
                        console.log("ListView count changed:", count)
                        console.log("ListView visible:", visible)
                        console.log("ListView contentHeight:", contentHeight)
                    }
                    
                    onModelChanged: {
                        console.log("ListView model changed, new count:", count)
                        forceLayout()
                    }

                    delegate: ListItem {
                        width: ListView.view ? ListView.view.width : parent.width
                        height: units.gu(7)
                        
                        Component.onCompleted: {
                            console.log("Delegate created for:", model.remoteAddress)
                        }
                        
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

