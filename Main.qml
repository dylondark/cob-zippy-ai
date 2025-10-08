import QtQuick
import QtQuick.VirtualKeyboard
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent

        Button {
            id: configButton
            text: "Ollama Config"
            onClicked: {
                const component = Qt.createComponent("OllamaConfig.qml")
                const win = component.createObject()
                if (win) win.show()
            }
        }

        TextArea {
            id: chatArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            readOnly: true
            wrapMode: TextArea.Wrap

            Connections {
                target: controller
                function onGenerateFinished(response) {
                    chatArea.append("Model: " + response)
                }
            }
        }

        RowLayout {
            id: inputLayout
            Layout.fillWidth: true

            TextField {
                id: inputField
                Layout.fillWidth: true
                placeholderText: "Type your message..."
                onAccepted: {
                    if (inputField.text.trim() !== "") {
                        chatArea.append("User: " + inputField.text)
                        controller.generate(inputField.text)
                        inputField.text = ""
                    }
                }
            }

            Button {
                id: sendButton
                text: "Send"
                onClicked: {
                    if (inputField.text.trim() !== "") {
                        chatArea.append("User: " + inputField.text)
                        controller.generate(inputField.text)
                        inputField.text = ""
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
