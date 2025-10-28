import QtQuick
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

        // This property tracks if the model is busy
        property bool isGenerating: false

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: window.height // Removed virtual keyboard adjustment

        Behavior on height {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }

        // Top toolbar with buttons and status
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            spacing: 10

            // Connection status indicator
            Rectangle {
                Layout.preferredWidth: 120
                Layout.fillHeight: true
                color: "#3c4049"
                radius: 5

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 5

                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: controller.getOllamaStatus() ? "#00ff00" : "#ff0000"
                    }

                    Text {
                        text: controller.getOllamaStatus() ? "Connected" : "Disconnected"
                        color: "white"
                        font.pixelSize: 12
                    }
                }
            }

            // Generate status indicator
            Rectangle {
                Layout.preferredWidth: 100
                Layout.fillHeight: true
                color: "#3c4049"
                radius: 5
                visible: mainLayout.isGenerating

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 5

                    // Animated loading indicator
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: "#4a9eff"

                        SequentialAnimation on opacity {
                            running: mainLayout.isGenerating
                            loops: Animation.Infinite
                            NumberAnimation { from: 1.0; to: 0.3; duration: 500 }
                            NumberAnimation { from: 0.3; to: 1.0; duration: 500 }
                        }
                    }

                    Text {
                        text: "Thinking..."
                        color: "white"
                        font.pixelSize: 12
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: clearButton
                text: "Clear Chat"
                Layout.preferredWidth: 100
                onClicked: {
                    chatArea.text = ""
                    controller.clearConversation()
                }
            }

            Button {
                id: configButton
                text: "Config"
                Layout.preferredWidth: 80
                onClicked: {
                    const component = Qt.createComponent("OllamaConfig.qml")
                    const win = component.createObject()
                    if (win) win.show()
                }
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


                    function onStreamFinished() {
                        mainLayout.isGenerating = false // Re-enable input
                    }

                    function onErrorOccurred(errorMessage) {
                        chatArea.append("\n[Error: " + errorMessage + "]")
                        mainLayout.isGenerating = false // Re-enable input on error
                    }
                }
            }
        }

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

                    //Disables text while Zippy is generating
                    enabled: !mainLayout.isGenerating
                    color: "white"
                    placeholderText: "Ask Zippy anything..."
                    font.pixelSize: 22
                    placeholderTextColor: "white"
                    activeFocusOnPress: true
                    onAccepted: {
                        // Disables button while enabled is false
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

                    // Disables button while Zippy is generating
                    enabled: !mainLayout.isGenerating

                    Layout.fillHeight: true
                    Layout.minimumWidth: 80
                    font.pixelSize: 22
                    font.bold: true

                    onClicked: {
                        if (inputField.text.trim() !== "") {
                            // Disabling Inputs
                            mainLayout.isGenerating = true

                            chatArea.append("User: " + inputField.text)
                            chatArea.append("")
                            chatArea.text += "Model: "
                            controller.generate(inputField.text)
                            inputField.text = ""

                            // Hides keyboards
                            chatFlickable.forceActiveFocus()
                        }
                    }
                }
            }
        }
    }

    // Virtual Keyboard - Commented out (requires Qt Virtual Keyboard module)
    // Uncomment this section after installing Qt Virtual Keyboard via Qt Maintenance Tool
    /*
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
    */
}
