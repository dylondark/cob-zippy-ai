import QtQuick
import QtQuick.VirtualKeyboard
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: window
    width: 1200
    height: 800
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: qsTr("Zippy AI")

    // This property tracks if the model is busy
    property bool isGenerating: false
    property bool showChat: false

    // Dark blue gradient background
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0a1628" }
            GradientStop { position: 1.0; color: "#1a2332" }
        }
    }

    // Main content area
    Item {
        anchors.fill: parent
        anchors.margins: 0

        // Header with Zippy branding
        Rectangle {
            id: header
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 300
            color: "transparent"

            RowLayout {
                anchors.centerIn: parent
                spacing: 40

                // Zippy mascot placeholder (will show image if available)
                Rectangle {
                    id: mascotContainer
                    width: 200
                    height: 200
                    color: "#2a3f5f"
                    radius: 100

                    Image {
                        id: mascotImage
                        anchors.fill: parent
                        anchors.margins: 10
                        source: "zippy_mascot.png"
                        fillMode: Image.PreserveAspectFit
                        visible: false
                        onStatusChanged: {
                            if (status === Image.Ready) {
                                visible = true
                                mascotPlaceholder.visible = false
                            }
                        }
                    }

                    // Placeholder if image doesn't exist
                    Text {
                        id: mascotPlaceholder
                        anchors.centerIn: parent
                        text: "ZIPPY"
                        font.pixelSize: 40
                        font.bold: true
                        color: "#ffd700"
                    }
                }

                // Title text
                Column {
                    spacing: 5

                    Text {
                        text: "ZIPPY AI"
                        font.pixelSize: 72
                        font.bold: true
                        color: "white"
                        font.family: "Arial"
                    }

                    Text {
                        text: "COLLEGE OF BUSINESS"
                        font.pixelSize: 28
                        font.letterSpacing: 3
                        color: "#a0a0a0"
                        font.family: "Arial"
                    }
                }
            }

            // Config button in top right
            Button {
                id: configButton
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 20
                text: "⚙ Settings"

                background: Rectangle {
                    color: configButton.hovered ? "#2a3f5f" : "#1a2f4f"
                    radius: 8
                    border.color: "#3a5f8f"
                    border.width: 1
                }

                contentItem: Text {
                    text: configButton.text
                    color: "white"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    const component = Qt.createComponent("OllamaConfig.qml")
                    const win = component.createObject()
                    if (win) win.show()
                }
            }
        }

        // Chat area (hidden initially)
        Rectangle {
            id: chatContainer
            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: inputContainer.top
            anchors.margins: 20
            anchors.topMargin: 0
            color: "#0f1923"
            radius: 15
            border.color: "#2a3f5f"
            border.width: 2
            visible: showChat

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        text: "Conversation"
                        font.pixelSize: 18
                        font.bold: true
                        color: "white"
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Clear Chat"

                        background: Rectangle {
                            color: parent.hovered ? "#3a2f2f" : "#2a1f1f"
                            radius: 6
                        }

                        contentItem: Text {
                            text: parent.text
                            color: "#ff6b6b"
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            chatArea.text = ""
                            showChat = false
                        }
                    }
                }

                Flickable {
                    id: chatFlickable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    contentHeight: chatArea.implicitHeight
                    contentWidth: width

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        width: 8
                    }

                    TextEdit {
                        id: chatArea
                        width: chatFlickable.width - 20
                        anchors.horizontalCenter: parent.horizontalCenter
                        readOnly: true
                        wrapMode: TextEdit.Wrap
                        color: "white"
                        font.pixelSize: 16
                        textFormat: TextEdit.RichText
                        selectionColor: "#4a6f9f"

                        Connections {
                            target: controller

                            function onGenerateFinished(response) {
                                chatArea.text += response
                                chatArea.cursorPosition = chatArea.length
                                chatFlickable.contentY = chatFlickable.contentHeight - chatFlickable.height
                            }

                            function onStreamFinished() {
                                chatArea.text += "</p>"
                                isGenerating = false
                            }

                            function onPromptParserError(error) {
                                chatArea.append("<p style='color: #ff6b6b;'>⚠ Error: " + error + "</p>")
                                isGenerating = false
                            }

                            function onSearchingWeb(query) {
                                chatArea.text += "<span style='color: #a0a0a0; font-style: italic;'>[Searching web...]</span> "
                            }
                        }
                    }
                }
            }
        }

        // Search/Input container
        Rectangle {
            id: inputContainer
            anchors.bottom: shortcutButtons.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 100
            anchors.rightMargin: 100
            anchors.bottomMargin: 30
            height: 80
            color: "#1a2f4f"
            radius: 40
            border.color: "#2a4f7f"
            border.width: 2

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 30
                anchors.rightMargin: 30
                spacing: 15

                TextField {
                    id: inputField
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    enabled: !isGenerating
                    color: "white"
                    placeholderText: "What events are happening this weekend?"
                    font.pixelSize: 20
                    placeholderTextColor: "#6a7a8a"
                    verticalAlignment: TextInput.AlignVCenter
                    activeFocusOnPress: false
                    inputMethodHints: Qt.ImhNoPredictiveText

                    background: Rectangle {
                        color: "transparent"
                    }

                    onAccepted: {
                        sendMessage()
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            inputField.forceActiveFocus()
                        }
                    }
                }

                // Keyboard button
                Button {
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 50

                    background: Rectangle {
                        color: parent.hovered ? "#2a4f7f" : "transparent"
                        radius: 25
                    }

                    contentItem: Text {
                        text: "⌨"
                        font.pixelSize: 24
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        inputField.forceActiveFocus()
                        Qt.inputMethod.show()
                    }
                }
            }
        }

        // Shortcut buttons
        Row {
            id: shortcutButtons
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: showChat ? 30 : 100
            spacing: 20
            visible: !showChat

            Behavior on anchors.bottomMargin {
                NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
            }

            Repeater {
                model: [
                    { text: "Building maps", icon: "🗺️" },
                    { text: "Directory", icon: "📋" },
                    { text: "Events", icon: "📅" },
                    { text: "Academic resources", icon: "📚" }
                ]

                Button {
                    width: 200
                    height: 70

                    background: Rectangle {
                        color: parent.hovered ? "#2a3f5f" : "#1a2f4f"
                        radius: 35
                        border.color: "#2a4f7f"
                        border.width: 2
                    }

                    contentItem: Text {
                        text: modelData.text
                        color: "white"
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                    }

                    onClicked: {
                        inputField.text = "Tell me about " + modelData.text.toLowerCase()
                        sendMessage()
                    }
                }
            }
        }

        // Send button (overlaid when text is entered)
        Button {
            id: sendButton
            anchors.right: inputContainer.right
            anchors.verticalCenter: inputContainer.verticalCenter
            anchors.rightMargin: 15
            width: 60
            height: 60
            visible: inputField.text.length > 0 || isGenerating

            background: Rectangle {
                color: isGenerating ? "#ff6b6b" : (parent.hovered ? "#3a7fc0" : "#2a6faf")
                radius: 30
            }

            contentItem: Text {
                text: isGenerating ? "⏹" : "➤"
                color: "white"
                font.pixelSize: 24
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                if (isGenerating) {
                    // Cancel generation
                    controller.cancelGeneration()
                    chatArea.append("<p style='color: #ffa500;'>[Generation cancelled]</p>")
                    isGenerating = false
                } else {
                    sendMessage()
                }
            }
        }
    }

    // Virtual keyboard
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

    function sendMessage() {
        if (inputField.text.trim() !== "") {
            showChat = true
            isGenerating = true

            chatArea.append("<p style='color: #4a9fff; font-weight: bold;'>You:</p>")
            chatArea.append("<p>" + inputField.text + "</p>")
            chatArea.text += "<p style='color: #ffd700; font-weight: bold;'>Zippy: "

            controller.generate(inputField.text)
            inputField.text = ""

            chatFlickable.contentY = chatFlickable.contentHeight - chatFlickable.height
        }
    }
}
