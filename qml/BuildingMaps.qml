import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    //Click count is for the easter egg
    property int clickCount: 0
    property string mapSource: "qrc:/images/MapFinalColor.png"
    property string secretSource: "qrc:/images/floor1_map.png"

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
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.clickCount++
                textPulse.start()

                if (root.clickCount === 5) {
                    easterEggAnimation.start()
                }
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

                // --- Floor 1 Map ---
                Image {
                    id: floor1Image
                    source: root.mapSource
                    fillMode: Image.PreserveAspectFit
                    opacity: 1.0

                    // Easter Egg Transition
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

                // Placeholder text for other floors
                Text { text: "Floor 2 Map\n(Coming Soon)"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: "#888" }
                Text { text: "Floor 3 Map\n(Coming Soon)"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: "#888" }
                Text { text: "Floor 4 Map\n(Coming Soon)"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: "#888" }
            }
        }
    }
}