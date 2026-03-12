import QtQuick

import QtQuick.Controls

import QtQuick.Layouts

Page {

    id: contactPage

    property color brandColor: "#0B1F8C"

    property color brandDark: "#08145F"

    property color pageBg: "#F3F5FB"

    property color cardBg: "#FFFFFF"

    property color softBlue: "#F6F8FF"

    property color borderColor: "#E5E7EB"

    property color textPrimary: "#111827"

    property color textSecondary: "#6B7280"

    property color fieldBg: "#F9FAFB"

    background: Rectangle {

        color: pageBg

    }

    Dialog {

        id: successDialog

        anchors.centerIn: parent

        modal: true

        title: "Ticket Submitted"

        standardButtons: Dialog.Ok

        contentItem: Text {

            text: "Your message has been saved successfully."

            wrapMode: Text.Wrap

            color: textPrimary

            font.pixelSize: window.sz(14)

            padding: window.sz(18)

        }

    }

    MouseArea {

        anchors.fill: parent

        onClicked: contactPage.forceActiveFocus()

    }

    header: Rectangle {

        width: parent.width

        height: window.sz(92)

        gradient: Gradient {

            GradientStop { position: 0.0; color: brandDark }

            GradientStop { position: 1.0; color: brandColor }

        }

        RowLayout {

            anchors.fill: parent

            anchors.leftMargin: window.sz(28)

            anchors.rightMargin: window.sz(28)

            Text {

                text: "Contact Support"

                color: "white"

                font.bold: true

                font.pixelSize: window.sz(24)

                Layout.alignment: Qt.AlignVCenter

            }

            Item {

                Layout.fillWidth: true

            }

        }

    }

    Flickable {

        id: flick

        anchors.fill: parent

        anchors.topMargin: header.height

        contentWidth: width

        contentHeight: contentWrapper.implicitHeight + window.sz(60)

        clip: true

        Item {

            id: contentWrapper

            width: flick.width

            implicitHeight: mainColumn.implicitHeight + window.sz(40)

            Column {

                id: mainColumn

                anchors.horizontalCenter: parent.horizontalCenter

                anchors.top: parent.top

                anchors.topMargin: window.sz(36)

                width: Math.min(parent.width * 0.92, window.sz(980))

                spacing: window.sz(22)

                Text {

                    width: parent.width

                    text: "We are here to help"

                    horizontalAlignment: Text.AlignHCenter

                    color: textPrimary

                    font.bold: true

                    font.pixelSize: window.sz(30)

                }

                Text {

                    width: parent.width

                    text: "Send your issue or question and our IT team will review it as soon as possible."

                    horizontalAlignment: Text.AlignHCenter

                    wrapMode: Text.Wrap

                    color: textSecondary

                    font.pixelSize: window.sz(14)

                }

                Rectangle {

                    id: mainCard

                    width: parent.width

                    radius: window.sz(22)

                    color: cardBg

                    border.color: "#EDF0F5"

                    implicitHeight: formLayout.implicitHeight + window.sz(40)

                    ColumnLayout {

                        id: formLayout

                        anchors.fill: parent

                        anchors.margins: window.sz(26)

                        spacing: window.sz(24)

                        RowLayout {

                            Layout.fillWidth: true

                            spacing: window.sz(24)

                            Rectangle {

                                Layout.preferredWidth: parent.width > window.sz(760) ? window.sz(300) : 0

                                Layout.fillHeight: true

                                Layout.alignment: Qt.AlignTop

                                visible: parent.width > window.sz(760)

                                radius: window.sz(18)

                                color: softBlue

                                border.color: "#E4EAFF"

                                Column {

                                    anchors.fill: parent

                                    anchors.margins: window.sz(22)

                                    spacing: window.sz(14)

                                    Text {

                                        width: parent.width

                                        text: "College of Business IT"

                                        wrapMode: Text.Wrap

                                        color: textPrimary

                                        font.bold: true

                                        font.pixelSize: window.sz(22)

                                    }

                                    Text {

                                        width: parent.width

                                        text: "Reach out for technical support, account access issues, website questions, or lab assistance."

                                        wrapMode: Text.Wrap

                                        color: textSecondary

                                        font.pixelSize: window.sz(14)

                                    }

                                    Rectangle {

                                        width: parent.width

                                        height: 1

                                        color: "#E5EAF8"

                                    }

                                    Text {

                                        width: parent.width

                                        text: "Email: cobzippyai@uakron.edu"

                                        wrapMode: Text.Wrap

                                        color: textPrimary

                                        font.pixelSize: window.sz(15)

                                    }

                                    Text {

                                        width: parent.width

                                        text: "Phone: (330) 972-7036"

                                        wrapMode: Text.Wrap

                                        color: textPrimary

                                        font.pixelSize: window.sz(15)

                                    }

                                    Text {

                                        width: parent.width

                                        text: "Location: Room 102"

                                        wrapMode: Text.Wrap

                                        color: textPrimary

                                        font.pixelSize: window.sz(15)

                                    }

                                }

                            }

                            ColumnLayout {

                                Layout.fillWidth: true

                                Layout.alignment: Qt.AlignTop

                                spacing: window.sz(16)

                                Text {

                                    text: "Submit a support ticket"

                                    color: textPrimary

                                    font.bold: true

                                    font.pixelSize: window.sz(22)

                                }

                                Text {

                                    text: "Please include enough detail so we can help faster."

                                    color: textSecondary

                                    font.pixelSize: window.sz(14)

                                }

                                ColumnLayout {

                                    Layout.fillWidth: true

                                    spacing: window.sz(8)

                                    Text {

                                        text: "Your name or email"

                                        color: textPrimary

                                        font.bold: true

                                        font.pixelSize: window.sz(13)

                                    }

                                    TextField {

                                        id: contactInput

                                        Layout.fillWidth: true

                                        Layout.preferredHeight: window.sz(52)

                                        placeholderText: "e.g. ZippyAI or name@uakron.edu"

                                        font.pixelSize: window.sz(14)

                                        selectByMouse: true

                                        padding: window.sz(14)

                                        background: Rectangle {

                                            radius: window.sz(12)

                                            color: fieldBg

                                            border.width: contactInput.activeFocus ? 2 : 1

                                            border.color: contactInput.activeFocus ? brandColor : borderColor

                                        }

                                    }

                                }

                                ColumnLayout {

                                    Layout.fillWidth: true

                                    spacing: window.sz(8)

                                    Text {

                                        text: "How can we help?"

                                        color: textPrimary

                                        font.bold: true

                                        font.pixelSize: window.sz(13)

                                    }

                                    TextArea {

                                        id: messageInput

                                        Layout.fillWidth: true

                                        Layout.preferredHeight: window.sz(180)

                                        placeholderText: "Describe the issue, what happened, and any steps we should know."

                                        wrapMode: TextEdit.Wrap

                                        font.pixelSize: window.sz(14)

                                        selectByMouse: true

                                        padding: window.sz(14)

                                        background: Rectangle {

                                            radius: window.sz(12)

                                            color: fieldBg

                                            border.width: messageInput.activeFocus ? 2 : 1

                                            border.color: messageInput.activeFocus ? brandColor : borderColor

                                        }

                                        Keys.onReturnPressed: function(event) {

                                            if ((event.modifiers & Qt.ShiftModifier) === 0) {

                                                submitButton.clicked()

                                                event.accepted = true

                                            } else {

                                                event.accepted = false

                                            }

                                        }

                                    }

                                }

                                RowLayout {

                                    Layout.fillWidth: true

                                    spacing: window.sz(12)

                                    Text {

                                        text: "Press Shift + Enter for a new line"

                                        color: textSecondary

                                        font.pixelSize: window.sz(12)

                                        Layout.fillWidth: true

                                    }

                                    Button {

                                        id: submitButton

                                        text: "Submit Ticket"

                                        Layout.preferredWidth: window.sz(180)

                                        Layout.preferredHeight: window.sz(50)

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

                                            radius: window.sz(14)

                                            color: submitButton.down ? brandDark : brandColor

                                        }

                                        contentItem: Text {

                                            text: submitButton.text

                                            color: "white"

                                            font.bold: true

                                            font.pixelSize: window.sz(15)

                                            horizontalAlignment: Text.AlignHCenter

                                            verticalAlignment: Text.AlignVCenter

                                        }

                                    }

                                }

                            }

                        }

                        Rectangle {

                            Layout.fillWidth: true

                            Layout.preferredHeight: window.sz(92)

                            visible: width <= window.sz(760)

                            radius: window.sz(16)

                            color: softBlue

                            border.color: "#E4EAFF"

                            Column {

                                anchors.fill: parent

                                anchors.margins: window.sz(16)

                                spacing: window.sz(6)

                                Text {

                                    text: "College of Business IT"

                                    color: textPrimary

                                    font.bold: true

                                    font.pixelSize: window.sz(16)

                                }

                                Text {

                                    text: "cobzippyai@uakron.edu   •   (330) 123-4567   •   Room 102"

                                    color: textSecondary

                                    font.pixelSize: window.sz(13)

                                    wrapMode: Text.Wrap

                                    width: parent.width

                                }

                            }

                        }

                    }

                }

            }

        }

    }

}

