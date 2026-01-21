/*==========================================================
 * Program : Main.qml              Project : ratatoskr
 * Author  : Michael Zanetti, Ian L., Philippe Andersson
 * Date    : 2026-01-21
 * Version : 0.0.4
 * Notice  : (c) Original work by Michael Zanetti, Canonical
 *           Adapted by Ian L. and Philippe Andersson
 * License : GNU GPL v3 or later
 * Comment : ContentHub shareplugin QML interface
 * Modification History:
 * - 2025-12-18 (0.0.1) : Adapted from ubtd-20.04.
 * - 2025-12-25 (0.0.3) : Switched to QQmlApplicationEngine.
 * - 2026-01-21 (0.0.4) : Fixed applicationName to match manifest.
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

    width: units.gu(100)
    height: units.gu(75)

    property var fileNames: []
    property bool peerSelected: false

    BtTransfer {
        id: transfer
    }

    Connections {
        target: ContentHub

        onShareRequested: {
            var tmp = []
            for (var i = 0; i < transfer.items.length; i++) {
                var filePath = String(transfer.items[i].url).replace('file://', '')
                print("Should share file", filePath)
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
        onDeviceDiscovered: console.log("New device: " + device)
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

                                property bool isInProgress: transfer.currentFile === modelData
                                property bool isDone: false
                                property bool isError: false
                                onIsInProgressChanged: {
                                    print("isinprogresschanged:", isInProgress, transfer.error)
                                    if (!isInProgress) {
                                        isDone = !transfer.error;
                                        isError = !!transfer.error
                                    }
                                }

                                ActivityIndicator {
                                    anchors.centerIn: parent
                                    visible: running
                                    running: !transfer.finished && transfer.progress > 0 && !parent.isDone
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

                ListView {
                    anchors.fill: parent
                    model: btModel
                    visible: !root.peerSelected
                    clip: true

                    delegate: ListItem {
                        ListItemLayout {
                            title.text: (deviceName ? deviceName : name)
                        }

                        onClicked: {
                            btModel.continuousDiscovery = false;
                            for (var i = 0; i < root.fileNames.length; i++) {
                                transfer.sendFile(remoteAddress, root.fileNames[i])
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
                        text: transfer.error ? "Transfer failed."
                                : transfer.finished ? (root.fileNames.length == 1 ? "File transferred." : "All files transferred.")
                                : "Transferring..."
                        fontSize: "large"
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        value: transfer.progress
                        visible: !transfer.finished && !transfer.error
                    }

                    Button {
                        Layout.fillWidth: true
                        text: i18n.tr("Close")
                        color: transfer.error ? LomiriColors.red : LomiriColors.green
                        onClicked: Qt.quit();
                        visible: transfer.finished || transfer.error
                    }
                }

            }
        }
    }
}

