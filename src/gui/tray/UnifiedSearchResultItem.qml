/*
 * Copyright (C) 2021 by Oleksandr Zolotov <alex@nextcloud.com>
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

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import Style

RowLayout {
    id: unifiedSearchResultItemDetails

    signal revealInFileManager(string localFullPath)

    property string title: ""
    property string subline: ""
    property string icons: ""
    property string iconPlaceholder: ""

    property bool iconsIsThumbnail: false
    property bool isRounded: false

    property int iconWidth: iconsIsThumbnail && icons !== "" ? Style.unifiedSearchResultIconWidth : Style.unifiedSearchResultIconWidth
    property int titleFontSize: Style.unifiedSearchResultTitleFontSize
    property int sublineFontSize: Style.unifiedSearchResultSublineFontSize

    property url resourceUrl: ""
    property string rootDir: ""
    property var syncDirectories: []

    property color titleColor: palette.buttonText
    property color sublineColor: palette.dark


    Accessible.role: Accessible.ListItem
    Accessible.name: resultTitle
    Accessible.onPressAction: unifiedSearchResultMouseArea.clicked()

    spacing: Style.trayHorizontalMargin

    Item {
        id: unifiedSearchResultImageContainer

        property int whiteSpace: (Style.trayListItemIconSize - unifiedSearchResultItemDetails.iconWidth)

        Layout.preferredWidth: unifiedSearchResultItemDetails.iconWidth
        Layout.preferredHeight: unifiedSearchResultItemDetails.iconWidth
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.leftMargin: Style.trayHorizontalMargin + (whiteSpace * (0.5 - Style.thumbnailImageSizeReduction))
        Layout.rightMargin: whiteSpace * (0.5 + Style.thumbnailImageSizeReduction)

        Image {
            id: unifiedSearchResultThumbnail
            anchors.fill: parent
            visible: false
            asynchronous: true
            source: "image://tray-image-provider/" + unifiedSearchResultItemDetails.icons
            cache: true
            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignHCenter
            sourceSize.width: width
            sourceSize.height: height
        }
        Rectangle {
            id: mask
            anchors.fill: unifiedSearchResultThumbnail
            visible: false
            radius: unifiedSearchResultItemDetails.isRounded ? width / 2 : 3
        }
        OpacityMask {
            id: imageData
            anchors.fill: unifiedSearchResultThumbnail
            visible: unifiedSearchResultItemDetails.icons !== ""
            source: unifiedSearchResultThumbnail
            maskSource: mask
        }
        Image {
            id: unifiedSearchResultThumbnailPlaceholder
            anchors.fill: parent
            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignHCenter
            cache: true
            source: "image://tray-image-provider/" + unifiedSearchResultItemDetails.iconPlaceholder
            visible: unifiedSearchResultItemDetails.iconPlaceholder !== "" && unifiedSearchResultItemDetails.icons === ""
            sourceSize.height: unifiedSearchResultItemDetails.iconWidth
            sourceSize.width: unifiedSearchResultItemDetails.iconWidth
        }
    }

    ListItemLineAndSubline {
        id: unifiedSearchResultTextContainer

        spacing: Style.standardSpacing

        Layout.fillWidth: true
        Layout.rightMargin: Style.trayHorizontalMargin

        lineText: unifiedSearchResultItemDetails.title.replace(/[\r\n]+/g, " ")
        sublineText: unifiedSearchResultItemDetails.subline.replace(/[\r\n]+/g, " ")
    }

    // Local icon button at the end of each search result
    Button {
        id: localButton
        visible: {
            if (unifiedSearchResultItemDetails.subline === "") {
                return false
            }
            try {
                var itemDir = unifiedSearchResultItemDetails.subline;
                // If subline contains a file, remove the filename and keep only the directory path
                if (itemDir.indexOf('/') !== -1) {
                    itemDir = itemDir.substring(0, itemDir.lastIndexOf('/') + 1);
                }

                var keys = Object.keys(unifiedSearchResultItemDetails.syncDirectories)
                for (var i = 0; i < keys.length; ++i) {
                    var remotePath = keys[i]
                    // Match exact directory or a proper descendant (remotePath + '/')
                    var normalizedRemotePath = remotePath.endsWith('/') ? remotePath : remotePath + '/';
                    if (itemDir.indexOf(normalizedRemotePath) === 0) {
                        return true
                    }
                }
                return false
            } catch (e) {
                return false
            }
        }
        icon.source: "image://svgimage-custom-color/folder-outline.svg/"
        leftPadding: 8  // Adjusted
        rightPadding: 8 // Adjusted
        topPadding: 8   // Adjusted
        bottomPadding: 8// Adjusted
        icon.width: Math.round(Math.min(width, height) * 0.9)
        icon.height: Math.round(Math.min(width, height) * 0.9)
        Layout.alignment: Qt.AlignVCenter
        HoverHandler { cursorShape: Qt.PointingHandCursor }

        background: Rectangle {
            radius: 8
            color: "lightgrey"
            border.color: "transparent"
        }

        onClicked: {
            if (unifiedSearchResultItemDetails.subline === "") return
            revealInFileManager(unifiedSearchResultItemDetails.subline)
        }
        ToolTip.text: "Open file location"
        ToolTip.visible: localButton.hovered
        Layout.preferredWidth: 32
        Layout.preferredHeight: 32
        Layout.minimumWidth: 32
        Layout.minimumHeight: 32
        Layout.maximumWidth: 32
        Layout.maximumHeight: 32
        Layout.fillWidth: false
    }

    // Web icon button at the end of each search result
    Button {
        id: webButton
        icon.source: "image://svgimage-custom-color/web-outline.svg/"
        leftPadding: 8  // Adjusted
        rightPadding: 8 // Adjusted
        topPadding: 8   // Adjusted
        bottomPadding: 8// Adjusted
        icon.width: Math.round(Math.min(width, height) * 0.9)
        icon.height: Math.round(Math.min(width, height) * 0.9)
        Layout.alignment: Qt.AlignVCenter
        HoverHandler { cursorShape: Qt.PointingHandCursor }

        background: Rectangle {
            radius: 8
            color: "lightgrey"
            border.color: "transparent"
        }

        onClicked: {
            if (unifiedSearchResultItemDetails.resourceUrl !== "") {
                Qt.openUrlExternally(unifiedSearchResultItemDetails.resourceUrl)
            }
        }
        ToolTip.text: "Open in web"
        ToolTip.visible: webButton.hovered
        Layout.preferredWidth: 32
        Layout.preferredHeight: 32
        Layout.minimumWidth: 32
        Layout.minimumHeight: 32
        Layout.maximumWidth: 32
        Layout.maximumHeight: 32
        Layout.fillWidth: false
        Layout.rightMargin: 16
    }


}
