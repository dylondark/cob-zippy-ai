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
        ListElement { title: "Graduate Programs Info Session"; date: "MAR 24"; time: "12:15 PM"; location: "Virtual" }
        ListElement { title: "Graduate Programs Info Session"; date: "APR 08"; time: "6:00 PM"; location: "Virtual" }
        ListElement { title: "MS in Economics Info Session"; date: "APR 14"; time: "6:00 PM"; location: "Virtual" }
        ListElement { title: "MS in Analytics/IS/SCM Info Session"; date: "APR 21"; time: "6:00 PM"; location: "Virtual" }
        ListElement { title: "2026 Ohio Economic Forum"; date: "APR 23"; time: "2:00 PM"; location: "Student Union" }
        ListElement { title: "Graduate Programs Info Session"; date: "APR 27"; time: "12:15 PM"; location: "Virtual" }
        ListElement { title: "Advancing Leaders Reception 2026"; date: "APR 28"; time: "4:00 PM"; location: "Student Union" }
        ListElement { title: "Graduate Programs Info Session"; date: "MAY 12"; time: "6:00 PM"; location: "Virtual" }
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
