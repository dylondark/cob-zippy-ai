import QtQuick
import QtQuick.VirtualKeyboard
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("Zippy AI")

    Rectangle {
        anchors.fill: parent
        color: "#070c72"
    }

    ColumnLayout {
        id: mainLayout
        spacing: 10


        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: window.height - (inputPanel.active ? inputPanel.height : 0)

        Behavior on height {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }

        Button {
            id: configButton
            text: "Ollama Config"
            Layout.alignment: Qt.AlignRight
            onClicked: {
                const component = Qt.createComponent("OllamaConfig.qml")
                const win = component.createObject()
                if (win) win.show()
            }
        }
        Flickable { //THIS IS WHERE THE ACTUAL RESPONSE IS SHOWN - Sage
            id: chatFlickable
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true // This keeps the text within the chat area!! - Sage
            contentHeight: chatArea.implicitHeight
            contentWidth: width
            ScrollBar.vertical: ScrollBar { // THIS IS THE SCROLLBAR - Sage
                policy: ScrollBar.AsNeeded
            }

            Rectangle { //Rectangle where chat is shown. Grows as more text is added -Sage
                width: chatFlickable.contentWidth
                height: chatFlickable.contentHeight
                color: "#00000033"
                radius: 5
            }


            TextEdit {
                id: chatArea
                width: chatFlickable.width
                height: implicitHeight
                readOnly: true
                wrapMode: TextEdit.Wrap
                color: "white"
                Connections {
                    target: controller
                    function onGenerateFinished(response) {
                        chatArea.text += response
                        chatArea.cursorPosition = chatArea.length
                    }
                }
            }
        }
        // Image {
        //     id: zippyImage
        //      source: "qrc:/images/zippy_photo.png"
        //      Layout.preferredWidth: 150
        //      Layout.preferredHeight: 150
        //      Layout.alignment: Qt.AlignHCenter
        //       fillMode: Image.PreserveAspectFit
        //  }

        Rectangle {
            id: inputBar
            Layout.fillWidth: true
            Layout.preferredHeight: 85
            color: "#282c34"

            RowLayout {
                id: inputLayout
                anchors.fill: parent
                anchors.leftMargin: 15
                anchors.rightMargin: 15
                spacing: 15

                TextField {
                    id: inputField
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    placeholderText: "Ask Zippy anything..."
                    font.pixelSize: 22
                    color: "white"
                    activeFocusOnPress: true
                    onAccepted: {
                        sendButton.clicked()
                    }
                    background: Rectangle {
                        color: "#3c4049"
                        border.color: "steelblue"
                        radius: 8
                    }
                }

                Button {
                    id: sendButton
                    text: "Send"
                    Layout.fillHeight: true
                    Layout.minimumWidth: 80
                    font.pixelSize: 22
                    font.bold: true

                    onClicked: {
                        if (inputField.text.trim() !== "") {
                            chatArea.append("User: " + inputField.text) // Repeats user text
                            chatArea.append("")
                            chatArea.text += "Model: " //This should only add Model once. We can change this to Zippy eventually -Sage
                            controller.generate(inputField.text)
                            inputField.text = ""
                            chatListView.forceActiveFocus()
                        }
                    }
                }
            }
        }
    }

    InputPanel {
        id: inputPanel
        z: 99
        x: 0
        y: window.height
        width: window.width

        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges {
                target: inputPanel
                y: window.height - inputPanel.height
            }
        }
        transitions: Transition {
            from: ""
            to: "visible"
            reversible: true
            ParallelAnimation {
                NumberAnimation {
                    properties: "y"
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
