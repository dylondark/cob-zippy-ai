import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    background: Rectangle { color: "#f5f5f7" }

    header: Rectangle {
        width: parent.width
        height: 60
        color: "#070c72"
        Text {
            anchors.centerIn: parent
            text: "Building Maps"
            color: "white"
            font.pixelSize: 20
            font.bold: true
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
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            radius: 15
            border.color: "#d0d0d0"

            StackLayout {
                anchors.fill: parent
                anchors.margins: 10
                currentIndex: mapTabs.currentIndex

                // Floor 1 Map
                Image {
                    source: "qrc:/images/floor1_map.png"
                    fillMode: Image.PreserveAspectFit

                    Text {
                        anchors.centerIn: parent
                        visible: parent.status !== Image.Ready
                        text: "Floor 1 Map\n(Image not found)"
                        horizontalAlignment: Text.AlignHCenter
                        color: "#888"
                    }
                }

                // Floor 2 Map (Placeholder)
                Text {
                    text: "Floor 2 Map\n(Coming Soon)"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: "#888"
                    font.pixelSize: 18
                }

                // Floor 3 Map (Placeholder)
                Text {
                    text: "Floor 3 Map\n(Coming Soon)"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: "#888"
                    font.pixelSize: 18
                }
            }
        }
    }
}
