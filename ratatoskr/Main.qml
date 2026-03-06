/*==========================================================
 * Program : Main.qml                    Project : ratatoskr
 * Author  : Michael Zanetti, Ian L., Philippe Andersson
 * Date    : 2026-03-06
 * Version : 0.0.3
 * Notice  : (c) Original work by Michael Zanetti, Canonical
 *           Adapted by Ian L. and Philippe Andersson
 * License : GNU GPL v3 or later
 * Comment : Main app QML interface
 * Modification History:
 * - 2025-12-18 (0.0.1) : Adapted from ubtd-20.04.
 * - 2026-02-17 (0.0.2) : Changed app author name to match GitHub account.
 * - 2026-03-06 (0.0.3) : Added incoming transfer confirmation dialog.
 *========================================================*/

import QtQuick 2.4
import QtQuick.Layouts 1.1
import Lomiri.Components 1.3
import Lomiri.Components.ListItems 1.3
import Lomiri.Components.Popups 1.3
import Lomiri.Content 1.3
import Ratatoskr 1.0

MainView {
    applicationName: "ratatoskr.petroniusniger"

    // Confirmation dialog for incoming Bluetooth transfers
    Component {
        id: incomingTransferDialog

        Dialog {
            id: dialogue
            property string transferPath: ""
            property string transferFilename: ""
            property string transferSize: ""

            title: i18n.tr("Incoming Bluetooth Transfer")
            text: i18n.tr("A device wants to send you a file.")

            Label {
                text: i18n.tr("File: <b>%1</b>").arg(dialogue.transferFilename)
                wrapMode: Text.WordWrap
                width: parent.width
            }
            Label {
                visible: dialogue.transferSize !== ""
                text: i18n.tr("Size: %1").arg(dialogue.transferSize)
                width: parent.width
            }

            Button {
                text: i18n.tr("Accept")
                color: theme.palette.normal.positive
                width: parent.width
                onClicked: {
                    obexd.acceptTransfer(dialogue.transferPath)
                    PopupUtils.close(dialogue)
                }
            }
            Button {
                text: i18n.tr("Reject")
                width: parent.width
                onClicked: {
                    obexd.rejectTransfer(dialogue.transferPath)
                    PopupUtils.close(dialogue)
                }
            }
        }
    }

    Connections {
        target: obexd
        onTransferNeedsConfirmation: {
            var sizeStr = ""
            if (size > 0) {
                if (size >= 1048576)
                    sizeStr = (size / 1048576).toFixed(1) + " MB"
                else if (size >= 1024)
                    sizeStr = (size / 1024).toFixed(1) + " KB"
                else
                    sizeStr = size + " B"
            }
            PopupUtils.open(incomingTransferDialog, null, {
                transferPath: path,
                transferFilename: filename,
                transferSize: sizeStr
            })
        }
    }


    width: units.gu(100)
    height: units.gu(75)

    PageStack {
        id: pageStack
        Component.onCompleted: push(mainPage)
    }

    Page {
        id: mainPage
        header: PageHeader {
            title: i18n.tr("Bluetooth File Transfer")
            trailingActionBar.actions: [
                Action {
                    iconName: "bluetooth-active"
                    text: i18n.tr("Send File")
                    onTriggered: pageStack.push(sendFilePage)
                }
            ]
        }

        ColumnLayout {
            spacing: units.gu(1)
            anchors {
                fill: parent
                topMargin: mainPage.header.height + units.gu(1)
            }

            Column {
                Layout.fillWidth: true
                Layout.preferredHeight: childrenRect.height
                spacing: units.gu(1)

                ColumnLayout {
                    width: parent.width - units.gu(4)
                    anchors.horizontalCenter: parent.horizontalCenter

                    Label {
                        id: label
                        text: i18n.tr("Received Files")
                        fontSize: "large"
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }
                    Label {
                        text: i18n.tr("Files sent to your Ubuntu Touch device via Bluetooth will appear below.")
                        fontSize: "small"
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }
                }
                ThinDivider {

                }
                Label {
                    text: i18n.tr("Transfers:")
                    anchors { left: parent.left; right: parent.right; margins: units.gu(2) }
                }
            }

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true

                model: obexd

                delegate: ListItem {
                    id: listItem
                    height: units.gu(10)

                    leadingActions: ListItemActions {
                        actions: [
                            Action {
                                iconName: "delete"
                                text: i18n.tr("Delete")
                                onTriggered: obexd.deleteFile(index)
                            }
                        ]
                    }

                    trailingActions: ListItemActions {
                        actions: [
                            Action {
                                iconName: "share"
                                text: i18n.tr("Share")
                                onTriggered: {
                                    pageStack.push(pickerPageComponent, {
                                        contentType: listItem.contentType,
                                        filePath: model.filePath + "/" + model.filename,
                                        pickerHandler: ContentHandler.Share
                                    })
                                }
                            },
                            Action {
                                iconName: "save"
                                text: i18n.tr("Save")
                                onTriggered: {
                                    pageStack.push(pickerPageComponent, {
                                        contentType: listItem.contentType,
                                        filePath: model.filePath + "/" + model.filename,
                                        pickerHandler: ContentHandler.Destination
                                    })
                                }
                            }
                        ]
                    }

                    property var contentType: null

                    RowLayout {
                        anchors { fill: parent; leftMargin: units.gu(2); rightMargin: units.gu(2); topMargin: units.gu(1); bottomMargin: units.gu(1) }


                        Item {
                            Layout.fillHeight: true
                            Layout.preferredWidth: height

                            Image {
                                id: transferredImage
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectCrop
                                source: model.status === Transfer.StatusComplete ? "file://" + filePath + "/" + filename : ""
                                visible: status === Image.Ready
                            }

                            Icon {
                                anchors.fill: parent
                                visible: status !== Transfer.StatusComplete || transferredImage.status === Image.Error
                                name: {
                                    var extension = filename.split(".").pop()
                                    switch(extension) {
                                    case "pdf":
                                        listItem.contentType = ContentType.Documents;
                                        return "application-pdf-symbolic";
                                    case "tar":
                                    case "gz":
                                    case "gzip":
                                    case "zip":
                                    case "xz":
                                        listItem.contentType = ContentType.Unknown;
                                        return "application-x-archive-symbolic";
                                    case "mp3":
                                    case "ogg":
                                    case "wav":
                                    case "flac":
                                        listItem.contentType = ContentType.Music;
                                        return "audio-x-generic-symbolic";
                                    case "jpg":
                                    case "gif":
                                    case "jpeg":
                                    case "png":
                                    case "webp":
                                        listItem.contentType = ContentType.Pictures;
                                        return "image-x-generic-symbolic";
                                    case "click":
                                    case "deb":
                                        listItem.contentType = ContentType.Unknown;
                                        return "package-x-generic-symbolic";
                                    case "txt":
                                        listItem.contentType = ContentType.Text;
                                        return "text-generic-symbolic";
                                    case "mp4":
                                    case "mkv":
                                    case "avi":
                                    case "mpeg":
                                    case "mpg":
                                        listItem.contentType = ContentType.Videos;
                                        return "video-x-generic-symbolic";
                                    default:
                                        listItem.contentType = ContentType.Unknown;
                                        return "empty-symbolic";
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            Label {
                                Layout.fillWidth: true
                                text: filename
                            }
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: units.dp(5)
                                visible: status == Transfer.StatusActive
                                color: theme.palette.normal.base
                                Rectangle {
                                    anchors.fill: parent
                                    color: UbuntuColors.blue
                                    // trans : total = x : width
                                    anchors.rightMargin: parent.width - (transferred * parent.width / size)
                                }
                            }
                            Label {
                                text: {
                                    switch (status) {
                                    case Transfer.StatusQueued:
                                        return "Waiting...";
                                    case Transfer.StatusActive:
                                        return "Transferring...";
                                    case Transfer.StatusSuspended:
                                        return "Paused"
                                    case Transfer.StatusComplete:
                                        return "Completed (" + Qt.formatDateTime(date) + ")";
                                    case Transfer.StatusError:
                                        return "Failed";
                                    }
                                    return "Uunknown";
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ContentItem {
        id: exportItem
        name: i18n.tr("Attachment")
    }

    Component {
        id: pickerPageComponent

        Page {
            id: pickerPage
            property alias contentType: picker.contentType
            property string filePath
            property int pickerHandler: ContentHandler.Destination

            head {
                visible: false
                locked: true
            }

            ContentPeerPicker {
                id: picker
                anchors.fill: parent
                handler: pickerPage.pickerHandler

                onPeerSelected: {
                     var transfer = peer.request();
                     if (transfer.state === ContentTransfer.InProgress) {
                         var items = new Array()
                         var path = "file://" + pickerPage.filePath;
                         exportItem.url = path
                         items.push(exportItem);
                         transfer.items = items;
                         transfer.stateChanged.connect(function() {
                             if (transfer.state == ContentTransfer.Finalized && pageStack.depth > 1) {
                                 pageStack.pop();
                             }
                         })
                         transfer.state = ContentTransfer.Charged;
                     }
                 }
                 onCancelPressed: pageStack.pop();
            }
        }
    }

    Component {
        id: sendFilePage

        Page {
            id: sendPage
            header: PageHeader {
                title: i18n.tr("Send File")
                trailingActionBar.actions: [
                    Action {
                        iconName: deviceDiscovery.discovering ? "media-playback-stop" : "search"
                        text: deviceDiscovery.discovering ? i18n.tr("Stop Scan") : i18n.tr("Scan")
                        onTriggered: {
                            if (deviceDiscovery.discovering) {
                                deviceDiscovery.stopDiscovery()
                            } else {
                                deviceDiscovery.startDiscovery()
                            }
                        }
                    }
                ]
            }

            ColumnLayout {
                anchors {
                    fill: parent
                    topMargin: sendPage.header.height + units.gu(2)
                    leftMargin: units.gu(2)
                    rightMargin: units.gu(2)
                }
                spacing: units.gu(2)

                Label {
                    text: i18n.tr("Nearby Devices")
                    fontSize: "large"
                    Layout.fillWidth: true
                }

                Label {
                    text: deviceDiscovery.discovering ? i18n.tr("Scanning for devices...") : i18n.tr("Tap scan to find nearby Bluetooth devices")
                    fontSize: "small"
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    visible: deviceDiscovery.count === 0
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: deviceDiscovery

                    delegate: ListItem {
                        height: units.gu(8)
                        divider.visible: false

                        onClicked: {
                            console.log("Selected device:", alias)
                        }

                        ListItemLayout {
                            title.text: alias || deviceName || address
                            subtitle.text: address
                            
                            Icon {
                                name: {
                                    if (connected) return "bluetooth-active"
                                    if (paired) return "bluetooth-active"
                                    return "bluetooth"
                                }
                                width: units.gu(4)
                                height: units.gu(4)
                                SlotsLayout.position: SlotsLayout.Leading
                            }

                            Row {
                                spacing: units.gu(1)
                                SlotsLayout.position: SlotsLayout.Trailing

                                Label {
                                    text: i18n.tr("Paired")
                                    fontSize: "small"
                                    visible: paired
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Icon {
                                    name: connected ? "network-cellular-connected" : ""
                                    width: units.gu(2)
                                    height: units.gu(2)
                                    visible: connected
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

