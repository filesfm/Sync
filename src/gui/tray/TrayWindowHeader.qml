/*
 * Copyright (C) 2020 by Dominique Fuchs <32204802+DominiqueFuchs@users.noreply.github.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../filedetails/"

import Style
import com.nextcloud.desktopclient

Rectangle {
    id: root

    readonly property alias currentAccountHeaderButton: currentAccountHeaderButton
    readonly property alias openLocalFolderButton: openLocalFolderButton
    readonly property alias appsMenu: appsMenu

    color: Style.currentUserHeaderColor

    palette {
        text: Style.currentUserHeaderTextColor
        windowText: Style.currentUserHeaderTextColor
        buttonText: Style.currentUserHeaderTextColor
        button: Style.adjustedCurrentUserHeaderColor
    }

    RowLayout {
        id: trayWindowHeaderLayout

        spacing: 0
        anchors.fill: parent

        CurrentAccountHeaderButton {
            id: currentAccountHeaderButton
            parentBackgroundColor: root.color
            Layout.preferredWidth: Style.currentAccountButtonWidth
            Layout.fillHeight: true
            Layout.leftMargin: 5
            Layout.topMargin: 5
            Layout.bottomMargin: 5

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                radius: 15
                border.width: 2
                border.color: "#575757"
            }
        }

        // Add space between items
        Item {
            Layout.fillWidth: true
        }

        HeaderButton {
            id: openLocalFolderButton
            icon.source: "image://svgimage-custom-color/folder.svg/" + palette.windowText

            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: Style.trayWindowHeaderHeight
            Layout.fillHeight: true

            visible: UserModel.currentUser && UserModel.currentUser.hasLocalFolder

            onClicked: {
                if (UserModel.currentUser.groupFolders.length > 0) {
                    // TODO: Implement menu for group folders
                    console.log("Group folders menu not implemented yet")
                } else {
                    UserModel.openCurrentAccountLocalFolder()
                }
            }

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("Open local folder")
            Accessible.onPressAction: openLocalFolderButton.clicked()
        }

        HeaderButton {
            id: trayWindowFeaturedAppButton

            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth:  Style.trayWindowHeaderHeight
            Layout.fillHeight: true

            visible: UserModel.currentUser.isFeaturedAppEnabled
            icon.source: UserModel.currentUser.featuredAppIcon + "/" + palette.windowText
            onClicked: UserModel.openCurrentAccountFeaturedApp()

            Accessible.role: Accessible.Button
            Accessible.name: UserModel.currentUser.featuredAppAccessibleName
            Accessible.onPressAction: trayWindowFeaturedAppButton.clicked() 
        }

        HeaderButton {
            id: trayWindowAppsButton
            // text: "Open in web"
            icon.source: "image://svgimage-custom-color/web.svg/" + palette.windowText

            onClicked: {
                UserModel.openCurrentAccountServer()
            }

            Accessible.role: Accessible.ButtonMenu
            Accessible.name: qsTr("Open in web")
            Accessible.onPressAction: trayWindowAppsButton.clicked()

            Menu {
                id: appsMenu
                x: Style.trayWindowMenuOffsetX
                y: (trayWindowAppsButton.y + trayWindowAppsButton.height + Style.trayWindowMenuOffsetY)
                width: Style.trayWindowWidth * Style.trayWindowMenuWidthFactor
                height: implicitHeight + y > Style.trayWindowHeight ? Style.trayWindowHeight - y : implicitHeight
                closePolicy: Menu.CloseOnPressOutsideParent | Menu.CloseOnEscape

                Repeater { 
                    model: UserAppsModel
                    delegate: MenuItem {
                        id: appEntry
                        // HACK: Without creating our own component (and killing native styling)
                        // HACK: we do not have a way to adjust the text and icon spacing.
                        text: "  " + model.appName
                        font.pixelSize: Style.topLinePixelSize
                        icon.source: model.appIconUrl
                        icon.color: palette.windowText
                        onTriggered: UserAppsModel.openAppUrl(appUrl)
                        Accessible.role: Accessible.MenuItem
                        Accessible.name: qsTr("Open %1 in browser").arg(model.appName)
                        Accessible.onPressAction: appEntry.triggered()
                    }
                }
            }
        }

        // Vertical separator
        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 2
            Layout.preferredHeight: parent.height * 0.6
            Layout.leftMargin: 10
            color: "#575757"
        }

        HeaderButton {
            id: chatButton
            icon.source: "image://svgimage-custom-color/ai-chat.svg/" + palette.windowText

            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: Style.trayWindowHeaderHeight
            Layout.fillHeight: true

            onClicked: Qt.openUrlExternally("https://chat.files.fm")

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("AI Chat")
            Accessible.onPressAction: chatButton.clicked()
        }
    }
}
