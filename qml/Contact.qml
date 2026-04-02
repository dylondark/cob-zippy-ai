import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: contactPage

    property color brandColor: "#0B1F8C"
    property color brandDark: "#08145F"
    property color pageBg: "#D6E0F0"
    property color cardBg: "#FFFFFF"
    property color softBlue: "#D2DEF2"
    property color borderColor: "#AEBED8"
    property color textPrimary: "#0F172A"
    property color textSecondary: "#5F6C82"
    property color fieldBg: "#FFFFFF"
    property color fieldHover: "#F7FAFF"
    property color successColor: "#0F8A5F"

    readonly property bool hasIdentity: contactInput.text.trim().length > 0
    readonly property bool hasMessage: messageInput.text.trim().length > 0
    readonly property bool canSubmit: hasMessage

    background: Rectangle {
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#E5ECF8" }
            GradientStop { position: 1.0; color: pageBg }
        }

        Rectangle {
            width: parent.width * 0.42
            height: parent.height * 0.2
            x: -width * 0.3
            y: parent.height * 0.24
            radius: width / 2
            color: "#C7D8F4"
            opacity: 0.3
        }
    }

    Dialog {
        id: successDialog
        anchors.centerIn: parent
        modal: true
        title: "Support request submitted"
        standardButtons: Dialog.Ok

        background: Rectangle {
            radius: window.sz(18)
            color: cardBg
            border.width: 1
            border.color: "#D7E0EC"
        }

        contentItem: Item {
            implicitWidth: dialogColumn.implicitWidth + window.sz(44)
            implicitHeight: dialogColumn.implicitHeight + window.sz(44)

            Column {
                id: dialogColumn
                anchors.fill: parent
                anchors.margins: window.sz(22)
                spacing: window.sz(10)

                Text {
                    width: parent.width
                    text: "Your message has been saved successfully."
                    wrapMode: Text.Wrap
                    color: textPrimary
                    font.bold: true
                    font.pixelSize: window.sz(16)
                }

                Text {
                    width: parent.width
                    text: "The College of Business IT team can now review it and follow up."
                    wrapMode: Text.Wrap
                    color: textSecondary
                    font.pixelSize: window.sz(13)
                }
            }
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        contentWidth: width
        contentHeight: contentWrapper.implicitHeight + window.sz(56)
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        Item {
            id: contentWrapper
            width: flick.width
            implicitHeight: headerSection.implicitHeight + mainColumn.implicitHeight + window.sz(48)

            Rectangle {
                id: headerSection
                width: parent.width
                height: window.sz(118)
                color: "#0B1F8C"

                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#112AA8" }
                    GradientStop { position: 1.0; color: brandColor }
                }

                Text {
                    anchors.centerIn: parent
                    text: "Contact Support"
                    color: "white"
                    font.bold: true
                    font.pixelSize: window.sz(26)
                }
            }

            Column {
                id: mainColumn
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: headerSection.bottom
                anchors.topMargin: window.sz(24)
                width: Math.min(parent.width * 0.92, window.sz(980))
                spacing: window.sz(20)
                opacity: 0
                y: window.sz(12)

                Behavior on opacity {
                    NumberAnimation {
                        duration: 240
                        easing.type: Easing.OutCubic
                    }
                }

                Behavior on y {
                    NumberAnimation {
                        duration: 280
                        easing.type: Easing.OutCubic
                    }
                }

                Component.onCompleted: {
                    opacity = 1
                    y = 0
                }

                Text {
                    width: parent.width
                    text: "Share the issue, what you expected, and any steps you already tried. A clear message helps the team respond faster."
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    color: textSecondary
                    font.pixelSize: window.sz(14)
                }

                Rectangle {
                    width: parent.width
                    radius: window.sz(24)
                    color: cardBg
                    border.width: 2
                    border.color: "#B9C8DE"
                    implicitHeight: formLayout.implicitHeight + window.sz(40)

                    Behavior on border.color {
                        ColorAnimation {
                            duration: 180
                        }
                    }

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
                                radius: window.sz(20)
                                color: softBlue
                                border.width: 2
                                border.color: "#B7CAE6"

                                Column {
                                    anchors.fill: parent
                                    anchors.margins: window.sz(22)
                                    spacing: window.sz(16)

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
                                        text: "Reach out for technical support, account access problems, website questions, or lab assistance."
                                        wrapMode: Text.Wrap
                                        color: textSecondary
                                        font.pixelSize: window.sz(14)
                                    }

                                    Rectangle {
                                        width: parent.width
                                        height: 1
                                        color: "#D8E4F8"
                                    }

                                    Column {
                                        width: parent.width
                                        spacing: window.sz(10)

                                        Text {
                                            text: "Email"
                                            color: textSecondary
                                            font.bold: true
                                            font.pixelSize: window.sz(12)
                                        }

                                        Text {
                                            text: "cobzippyai@uakron.edu"
                                            color: textPrimary
                                            font.pixelSize: window.sz(15)
                                        }

                                        Text {
                                            text: "Phone"
                                            color: textSecondary
                                            font.bold: true
                                            font.pixelSize: window.sz(12)
                                        }

                                        Text {
                                            text: "(330) 972-7036"
                                            color: textPrimary
                                            font.pixelSize: window.sz(15)
                                        }

                                        Text {
                                            text: "Location"
                                            color: textSecondary
                                            font.bold: true
                                            font.pixelSize: window.sz(12)
                                        }

                                        Text {
                                            text: "Room 102"
                                            color: textPrimary
                                            font.pixelSize: window.sz(15)
                                        }
                                    }
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignTop
                                spacing: window.sz(16)

                                RowLayout {
                                    Layout.fillWidth: true

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: window.sz(6)

                                        Text {
                                            text: "Submit a support ticket"
                                            color: textPrimary
                                            font.bold: true
                                            font.pixelSize: window.sz(22)
                                        }

                                        Text {
                                            text: "Please include enough detail so we can understand the problem on the first review."
                                            color: textSecondary
                                            font.pixelSize: window.sz(14)
                                            wrapMode: Text.Wrap
                                            Layout.fillWidth: true
                                        }
                                    }

                                    Rectangle {
                                        visible: hasMessage
                                        radius: window.sz(12)
                                        color: "#EAF7F1"
                                        border.width: 1
                                        border.color: "#CBE8DA"
                                        Layout.preferredHeight: window.sz(34)
                                        Layout.preferredWidth: window.sz(118)

                                        Text {
                                            anchors.centerIn: parent
                                            text: "Ready to submit"
                                            color: successColor
                                            font.bold: true
                                            font.pixelSize: window.sz(12)
                                        }
                                    }
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
                                        Layout.preferredHeight: window.sz(54)
                                        hoverEnabled: true
                                        placeholderText: "e.g. ZippyAI or name@uakron.edu"
                                        placeholderTextColor: "#8693A7"
                                        color: textPrimary
                                        font.pixelSize: window.sz(14)
                                        selectByMouse: true
                                        padding: window.sz(14)

                                        background: Rectangle {
                                            radius: window.sz(14)
                                            color: contactInput.hovered || contactInput.activeFocus ? fieldHover : fieldBg
                                            border.width: contactInput.activeFocus ? 2 : 2
                                            border.color: contactInput.activeFocus ? brandColor : borderColor

                                            Behavior on color {
                                                ColorAnimation { duration: 140 }
                                            }

                                            Behavior on border.color {
                                                ColorAnimation { duration: 160 }
                                            }
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
                                        Layout.preferredHeight: window.sz(190)
                                        hoverEnabled: true
                                        placeholderText: "Describe the issue, what happened, and any steps we should know."
                                        placeholderTextColor: "#8693A7"
                                        color: textPrimary
                                        wrapMode: TextEdit.Wrap
                                        font.pixelSize: window.sz(14)
                                        selectByMouse: true
                                        padding: window.sz(14)

                                        background: Rectangle {
                                            radius: window.sz(14)
                                            color: messageInput.hovered || messageInput.activeFocus ? fieldHover : fieldBg
                                            border.width: messageInput.activeFocus ? 2 : 2
                                            border.color: messageInput.activeFocus ? brandColor : borderColor

                                            Behavior on color {
                                                ColorAnimation { duration: 140 }
                                            }

                                            Behavior on border.color {
                                                ColorAnimation { duration: 160 }
                                            }
                                        }

                                        Keys.onReturnPressed: function(event) {
                                            if ((event.modifiers & Qt.ShiftModifier) === 0 && canSubmit) {
                                                submitButton.clicked()
                                                event.accepted = true
                                            } else {
                                                event.accepted = false
                                            }
                                        }
                                    }

                                    Text {
                                        text: hasMessage
                                              ? "Thanks. This has enough content to submit."
                                              : "Add a short description of the issue to enable submission."
                                        color: hasMessage ? successColor : textSecondary
                                        font.pixelSize: window.sz(12)
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
                                        enabled: canSubmit
                                        Layout.preferredWidth: window.sz(184)
                                        Layout.preferredHeight: window.sz(52)
                                        hoverEnabled: true

                                        onClicked: {
                                            if (!canSubmit) {
                                                return
                                            }

                                            var who = contactInput.text.trim()
                                            var msg = messageInput.text.trim()
                                            var success = contactSupport.saveTicket(who, msg)

                                            if (success) {
                                                successDialog.open()
                                                messageInput.text = ""
                                                contactInput.text = ""
                                                contactPage.forceActiveFocus()
                                            }
                                        }

                                        background: Rectangle {
                                            radius: window.sz(14)
                                            color: !submitButton.enabled ? "#A8B4D0"
                                                  : submitButton.down ? brandDark
                                                  : submitButton.hovered ? "#1733B0"
                                                  : brandColor
                                            scale: submitButton.down ? 0.985 : 1.0

                                            Behavior on color {
                                                ColorAnimation { duration: 140 }
                                            }

                                            Behavior on scale {
                                                NumberAnimation {
                                                    duration: 120
                                                    easing.type: Easing.OutCubic
                                                }
                                            }
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
                            Layout.preferredHeight: window.sz(104)
                            visible: width <= window.sz(760)
                            radius: window.sz(18)
                            color: softBlue
                            border.width: 2
                            border.color: "#B7CAE6"

                            Column {
                                anchors.fill: parent
                                anchors.margins: window.sz(16)
                                spacing: window.sz(8)

                                Text {
                                    text: "College of Business IT"
                                    color: textPrimary
                                    font.bold: true
                                    font.pixelSize: window.sz(16)
                                }

                                Text {
                                    text: "cobzippyai@uakron.edu"
                                    color: textPrimary
                                    font.pixelSize: window.sz(13)
                                }

                                Text {
                                    text: "(330) 972-7036   |   Room 102"
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
