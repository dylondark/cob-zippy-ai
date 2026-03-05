import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    background: Rectangle { color: "#f5f5f7" }

    header: Rectangle {
        width: parent.width
        height: window.sz(60) // Scaled
        color: "#070c72"
        Text {
            anchors.centerIn: parent
            text: "Upcoming Events"
            color: "white"
            font.pixelSize: window.sz(20) // Scaled
            font.bold: true
        }
    }

    ListModel {
        id: eventModel
        ListElement { title: "Sales and You: How to Close"; date: "MAR 08"; time: "10:00 AM"; location: "Grand Hall" }
        ListElement { title: "Robotic Business: AI Businessmen"; date: "MAR 15"; time: "2:00 PM"; location: "Room 304" }
        ListElement { title: "Professional Career Opportunities"; date: "APR 20"; time: "6:00 PM"; location: "Student Union" }
    }

    ListView {
        id: eventList
        anchors.fill: parent
        anchors.margins: window.sz(20) // Scaled
        spacing: window.sz(15) // Scaled
        model: eventModel
        clip: true

        delegate: Rectangle {
            width: eventList.width
            height: window.sz(100) // Scaled
            radius: window.sz(12) // Scaled
            color: "white"
            border.color: "#e0e0e0"

            RowLayout {
                anchors.fill: parent
                spacing: 0

                // Date Box
                Rectangle {
                    Layout.preferredWidth: window.sz(80) // Scaled
                    Layout.fillHeight: true
                    color: "#070c72"
                    radius: window.sz(12) // Scaled

                    // Small overlap fix to keep right corners square against the content
                    Rectangle {
                        width: window.sz(12)
                        height: parent.height
                        anchors.right: parent.right
                        color: "#070c72"
                    }

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: window.sz(2)
                        Text {
                            text: model.date.split(" ")[0]
                            color: "white"; font.pixelSize: window.sz(12) // Scaled
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Text {
                            text: model.date.split(" ")[1]
                            color: "white"; font.pixelSize: window.sz(22); font.bold: true // Scaled
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.margins: window.sz(15) // Scaled
                    spacing: window.sz(5) // Scaled

                    Text {
                        text: model.title
                        font.bold: true; font.pixelSize: window.sz(18) // Scaled
                        color: "#333"
                    }
                    Text {
                        text: "🕒 " + model.time + "  📍 " + model.location
                        color: "#666"; font.pixelSize: window.sz(14) // Scaled
                    }
                }
            }
        }
    }
}
