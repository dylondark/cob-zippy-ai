import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    property int clickCount: 0
    property bool showLegacyAdminButton: false
    property string mapSource: "qrc:/images/MapFinalColor.png"
    property string secretSource: "qrc:/images/floor1_map.png"

    Timer {
        id: clickResetTimer
        interval: 1800
        repeat: false
        onTriggered: root.clickCount = 0
    }

    background: Rectangle { color: "#f5f5f7" }

    header: Rectangle {
        id: headerBar
        width: parent.width
        height: 60
        color: "#070c72"

        Text {
            id: headerText
            anchors.centerIn: parent
            text: "Building Maps"
            color: "white"
            font.pixelSize: 20
            font.bold: true

            SequentialAnimation on scale {
                id: textPulse
                running: false
                NumberAnimation { to: 1.05; duration: 50 }
                NumberAnimation { to: 1.0; duration: 50 }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.clickCount++
                    clickResetTimer.restart()
                    textPulse.start()

                    if (root.clickCount >= 5) {
                        root.clickCount = 0
                        clickResetTimer.stop()
                        root.showLegacyAdminButton = true
                        easterEggAnimation.start()
                    }
                }
            }
        }

        Button {
            id: legacyMapButton
            visible: root.showLegacyAdminButton
            z: 2
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 16
            width: 108
            height: 40
            text: "Admin"
            font.pixelSize: 14
            font.bold: true
            onClicked: {
                mapTabs.currentIndex = 0
                if (root.mapSource !== root.secretSource) {
                    easterEggAnimation.start()
                }
                const component = Qt.createComponent("OllamaConfig.qml")
                const win = component.createObject(root)
                if (win) {
                    win.show()
                }
            }

            background: Rectangle {
                color: legacyMapButton.hovered ? "#2342b2" : "#10237f"
                radius: 10
                border.color: "#7d92ff"
                border.width: 1
            }

            contentItem: Text {
                text: legacyMapButton.text
                color: "white"
                font.pixelSize: legacyMapButton.font.pixelSize
                font.bold: legacyMapButton.font.bold
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        TabBar {
            id: mapTabs
            Layout.fillWidth: true
            background: Rectangle { color: "transparent" }

            TabButton { text: "Floor 1" }
            TabButton { text: "Floor 2" }
            TabButton { text: "Floor 3" }
            TabButton { text: "Floor 4" }
        }

        Rectangle {
            id: mapContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            radius: 15
            border.color: "#d0d0d0"
            clip: true

            StackLayout {
                anchors.fill: parent
                anchors.margins: 10
                currentIndex: mapTabs.currentIndex

                Image {
                    id: floor1Image
                    source: root.mapSource
                    fillMode: Image.PreserveAspectFit
                    opacity: 1.0

                    SequentialAnimation {
                        id: easterEggAnimation

                        ParallelAnimation {
                            NumberAnimation { target: floor1Image; property: "opacity"; to: 0; duration: 400; easing.type: Easing.OutCubic }
                            NumberAnimation { target: floor1Image; property: "verticalAlignment"; to: Image.AlignBottom; duration: 400 }
                        }
                        PropertyAction { target: root; property: "mapSource"; value: root.secretSource }
                        ParallelAnimation {
                            NumberAnimation { target: floor1Image; property: "opacity"; to: 1.0; duration: 600; easing.type: Easing.OutCubic }
                            NumberAnimation { target: floor1Image; property: "verticalAlignment"; to: Image.AlignCenter; duration: 600 }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        visible: parent.status !== Image.Ready
                        text: "Floor 1 Map\n(Image not found)"
                        horizontalAlignment: Text.AlignHCenter
                        color: "#888"
                    }
                }

                Text { text: "Floor 2 Map\n(Coming Soon)"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: "#888" }
                Text { text: "Floor 3 Map\n(Coming Soon)"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: "#888" }
                Text { text: "Floor 4 Map\n(Coming Soon)"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: "#888" }
            }
        }
    }
}
