import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: contactPage
    background: Rectangle { color: "#f5f5f7" }

    // A simple popup to confirm the save worked
    Dialog {
        id: successDialog
        anchors.centerIn: parent
        title: "Ticket Submitted"
        standardButtons: Dialog.Ok
        modal: true

        Text {
            text: "Your message has been saved successfully."
            anchors.centerIn: parent
            leftPadding: window.sz(20) // Scaled
            rightPadding: window.sz(20) // Scaled
            font.pointSize: window.sz(11) // Scaled
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: contactPage.forceActiveFocus()
    }

    header: Rectangle {
        width: parent.width
        height: window.sz(60) // Scaled
        color: "#070c72"
        Text {
            anchors.centerIn: parent
            text: "Contact Support"
            color: "white"
            font.pixelSize: window.sz(20) // Scaled
            font.bold: true
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        // Scale the max width logic as well
        width: Math.min(parent.width * 0.9, window.sz(400))
        spacing: window.sz(15) // Scaled

        // --- TOP INFO BOX ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: window.sz(150) // Scaled
            radius: window.sz(15) // Scaled
            color: "white"
            border.color: "#e0e0e0"

            ColumnLayout {
                anchors.centerIn: parent
                spacing: window.sz(10) // Scaled
                Text { text: "College of Business IT"; font.bold: true; font.pixelSize: window.sz(18); color: "#070c72" }
                Rectangle { height: 1; width: window.sz(100); color: "#eee" }
                Text { text: "zipAIsupport@uakron.edu"; font.pixelSize: window.sz(16) }
                Text { text: "(330) 123-4567"; font.pixelSize: window.sz(16) }
                Text { text: "Room 107"; font.pixelSize: window.sz(16) }
            }
        }

        // --- CONTACT INFO FIELD ---
        Text {
            text: "Your Name / Email (Optional):"
            font.bold: true
            color: "#555"
            font.pixelSize: window.sz(13) // Added scaling
            Layout.topMargin: window.sz(10)
        }

        TextField {
            id: contactInput
            Layout.fillWidth: true
            Layout.preferredHeight: window.sz(45) // Added scaling
            placeholderText: "e.g., Zippy / zippy@uakron.edu"
            font.pixelSize: window.sz(14)

            background: Rectangle {
                color: "white"
                border.color: "#ccc"
                radius: window.sz(8)
            }
        }

        // --- MESSAGE FIELD ---
        Text {
            text: "How can we help?"
            font.bold: true
            color: "#555"
            font.pixelSize: window.sz(13)
            Layout.topMargin: window.sz(5)
        }

        TextArea {
            id: messageInput
            Layout.fillWidth: true
            Layout.preferredHeight: window.sz(100) // Scaled
            placeholderText: "Type your issue here... (Enter to send)"
            wrapMode: Text.Wrap
            font.pixelSize: window.sz(14)

            background: Rectangle {
                color: "white"
                border.color: "#ccc"
                radius: window.sz(8)
            }

            Keys.onReturnPressed: (event) => {
                if ((event.modifiers & Qt.ShiftModifier) == 0) {
                    submitButton.clicked()
                    event.accepted = true
                } else {
                    event.accepted = false
                }
            }
        }

        // --- SUBMIT BUTTON ---
        Button {
            id: submitButton
            text: "Submit Ticket"
            Layout.fillWidth: true
            Layout.preferredHeight: window.sz(45) // Scaled

            onClicked: {
                if (messageInput.text.trim() !== "") {
                    var who = contactInput.text.trim()
                    var msg = messageInput.text.trim()
                    var success = contactSupport.saveTicket(who, msg)

                    if (success) {
                        successDialog.open()
                        messageInput.text = ""
                        contactInput.text = ""
                    }
                }
            }

            background: Rectangle {
                color: parent.down ? "#050950" : "#070c72"
                radius: window.sz(8)
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                font.bold: true
                font.pixelSize: window.sz(16) // Scaled
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
