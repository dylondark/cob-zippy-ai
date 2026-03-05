import QtQuick
import QtQuick.VirtualKeyboard
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: window
    width: Screen.width * 0.6
    height: Screen.height * 0.6
    visible: true
    visibility: Window.Maximized
    title: qsTr("Zippy AI")

    readonly property real scaleFactor: Math.min(window.width / 1000, window.height / 800)

    function sz(x) {
        return Math.round(x * scaleFactor);
    }

    // Data model to store chat messages
    // Kept at Window level so chat persists when navigating between tabs
    ListModel {
        id: chatModel
    }

    Rectangle {
        anchors.fill: parent
        color: "#070c72"
    }

    Component.onCompleted: {
        // Safe check ensures UI loads even if C++ controller isn't ready
        if (typeof controller !== "undefined") {
            controller.pingOllama();
        }
    }

    ColumnLayout {
        id: mainLayout
        spacing: 0
        anchors.fill: parent
        anchors.bottomMargin: inputPanel.active ? inputPanel.height : 0
        property bool isGenerating: false

        Behavior on anchors.bottomMargin {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }

        // ===== HEADER BAR (Always Visible) =====
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: sz(80)
            color: "#070c72"
            z: 10
            RowLayout {
                anchors.fill: parent
                anchors.margins: sz(20)
                Text {
                    text: "💬 Zippy AI — College of Business Assistant"
                    color: "white"
                    font.pointSize: sz(14)
                    font.bold: true
                    Layout.fillWidth: true
                }
                Button {
                    id: configButton
                    text: "⚙"
                    font.pointSize: sz(18)
                    Layout.preferredWidth: sz(60)
                    Layout.preferredHeight: sz(50)
                    onClicked: {
                        const component = Qt.createComponent("OllamaConfig.qml")
                        const win = component.createObject()
                        if (win) win.show()
                    }
                    background: Rectangle {
                        color: configButton.hovered ? "#0a0f8f" : "transparent"
                        radius: sz(10)
                    }
                }
            }
        }

        // ===== CONTENT AREA (StackView) =====
        // This handles switching between Chat, Maps, Events, etc.
        StackView {
            id: contentStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: homePage
            replaceEnter: Transition { PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 } }
            replaceExit: Transition { PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 } }
        }

        // ===== FOOTER NAV BAR (Always Visible) =====
        Rectangle {
            id: navigationBar
            Layout.fillWidth: true
            Layout.preferredHeight: sz(90)
            color: "#070c72"
            z: 10

            RowLayout {
                anchors.fill: parent
                anchors.margins: sz(20)
                spacing: sz(15)

                // Reusable Nav Button Component
                component NavButton: Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: sz(55)
                    background: Rectangle {
                        color: parent.down ? "#4040ff" : (parent.hovered ? "#2323ff" : "#1a1f6b")
                        radius: sz(12)
                        Behavior on color { ColorAnimation { duration: 150 } }
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pointSize: sz(11)
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                NavButton { text: "Home Page"; onClicked: contentStack.replace(homePage) }
                NavButton {
                    text: "Building Maps"
                    // Loads external BuildingMaps.qml
                    onClicked: contentStack.replace("BuildingMaps.qml")
                }
                NavButton { text: "Events"; onClicked: contentStack.replace("Events.qml") }
                NavButton { text: "Contact"; onClicked: contentStack.replace("Contact.qml") }
            }
        }
    }

    // =============================================
    // ===== COMPONENT: HOME PAGE (Chat Logic) =====
    // =============================================
    Component {
        id: homePage
        ColumnLayout {
            spacing: 0
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#fffaa0" }
                    GradientStop { position: 1.0; color: "#2323ff" }
                }

                RowLayout {
                    visible: chatModel.count === 0
                    anchors.fill: parent
                    anchors.margins: sz(40)
                    spacing: sz(50)

                    Image {
                        source: "qrc:/images/ZippyAILogo.png"
                        Layout.preferredWidth: sz(450)
                        Layout.preferredHeight: sz(450)
                        fillMode: Image.PreserveAspectFit
                        Layout.alignment: Qt.AlignBottom
                        smooth: true
                        mipmap: true
                        antialiasing: true
                    }

                    Text {
                        text: "ZIPPY AI\nCOLLEGE OF BUSINESS"
                        color: "#070c72"
                        font.pointSize: sz(32)
                        font.bold: true
                        horizontalAlignment: Text.AlignLeft
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                ListView {
                    id: chatListView
                    anchors.fill: parent
                    anchors.margins: sz(20)
                    spacing: sz(15)
                    clip: true
                    model: chatModel
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    delegate: Item {
                        width: chatListView.width
                        height: Math.max(messageBubble.height + sz(8), sz(58))
                        property real avatarTopY: height - sz(8) - sz(50)

                        Row {
                            anchors.right: model.isUser ? parent.right : undefined
                            anchors.left: model.isUser ? undefined : parent.left
                            spacing: sz(8)

                            // Avatar
                            Item {
                                visible: !model.isUser
                                width: sz(50); height: sz(50)
                                anchors.bottom: parent.bottom
                                Rectangle {
                                    anchors.fill: parent
                                    radius: sz(20); color: "white"; clip: true
                                    border.color: "#e0e0e0"; border.width: 1
                                    Image {
                                        source: "qrc:/images/ZippyAvatar.png"
                                        anchors.centerIn: parent
                                        width: parent.width - sz(4); height: parent.height - sz(4)
                                        fillMode: Image.PreserveAspectFit
                                        smooth: true; mipmap: true; antialiasing: true
                                    }
                                }
                            }

                            // Message Bubble
                            Rectangle {
                                id: messageBubble
                                width: Math.min(messageText.implicitWidth + sz(40), chatListView.width * 0.7)
                                height: messageText.implicitHeight + sz(30)
                                radius: sz(25)
                                color: model.isUser ? "#80007AFF" : "#803a3a3c"
                                border.color: model.isUser ? "#99FFFFFF" : "#77FFFFFF"
                                border.width: 1.5

                                Text {
                                    id: messageText
                                    text: model.message
                                    textFormat: Text.MarkdownText
                                    color: "white"
                                    wrapMode: Text.Wrap
                                    anchors.fill: parent; anchors.margins: sz(15)
                                    font.pointSize: sz(11)
                                    onLinkActivated: (link) => Qt.openUrlExternally(link)
                                }
                            }
                        }

                        // Thinking Indicator
                        Item {
                            id: zippyThinkingIndicator
                            visible: !model.isUser && mainLayout.isGenerating && index === chatModel.count - 1 && model.message === ""
                            width: sz(30)
                            height: sz(25)
                            z: 100
                            x: sz(15) + sz(50) - sz(30) - sz(5)
                            y: avatarTopY - sz(15)

                            Image {
                                id: thinkingIcon
                                anchors.centerIn: parent
                                width: sz(30); height: sz(30)
                                fillMode: Image.PreserveAspectFit
                                source: "qrc:/images/thought.png"
                                smooth: true
                                mipmap: true
                                SequentialAnimation {
                                    running: true
                                    loops: Animation.Infinite
                                    NumberAnimation { target: thinkingIcon; property: "opacity"; from: 1.0; to: 0.4; duration: 800; easing.type: Easing.InOutQuad }
                                    NumberAnimation { target: thinkingIcon; property: "opacity"; from: 0.4; to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                                }
                            }
                        }
                    }
                    onCountChanged: Qt.callLater(positionViewAtEnd)
                }
            }

            // Input Bar
            Rectangle {
                id: inputBar
                Layout.fillWidth: true
                Layout.preferredHeight: sz(130)
                color: "#070c72"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: sz(15)
                    spacing: sz(5)

                    RowLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        spacing: sz(15)

                        Button {
                            id: clearChatButton
                            text: "Clear Chat"
                            Layout.preferredWidth: sz(120); Layout.preferredHeight: sz(60)
                            font.pointSize: sz(10); font.bold: true
                            enabled: chatModel.count > 0 && !mainLayout.isGenerating
                            onClicked: chatModel.clear()
                            background: Rectangle { radius: height/2; color: clearChatButton.enabled ? "#8B0000" : "#5a5a5a" }
                        }

                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: sz(60)
                            radius: height/2; color: "#1a1f6b"; border.color: "#4a4f9b"; border.width: 2
                            TextField {
                                id: inputField
                                anchors.fill: parent; anchors.leftMargin: sz(25); anchors.rightMargin: sz(25)
                                enabled: !mainLayout.isGenerating
                                placeholderTextColor: "white"
                                font.pointSize: sz(12)
                                placeholderText: "Ask Zippy anything..."
                                verticalAlignment: TextInput.AlignVCenter
                                background: Item {}
                                onAccepted: sendButton.clicked()

                                Connections {
                                    target: (typeof controller !== "undefined") ? controller : null
                                    function onGenerateFinished(response) {
                                        if (chatModel.count > 0) {
                                            var lastIndex = chatModel.count - 1
                                            var lastMsg = chatModel.get(lastIndex)
                                            if (!lastMsg.isUser) {
                                                chatModel.setProperty(lastIndex, "message", lastMsg.message + response)
                                            }
                                        }
                                    }
                                    function onStreamFinished() { mainLayout.isGenerating = false }
                                }
                            }
                        }

                        Button {
                            id: sendButton
                            text: "↑"
                            enabled: !mainLayout.isGenerating && inputField.text.trim() !== ""
                            Layout.preferredWidth: sz(60); Layout.preferredHeight: sz(60)
                            font.pointSize: sz(18)
                            onClicked: {
                                var trimmedText = inputField.text.trim()
                                if (trimmedText === "") return
                                mainLayout.isGenerating = true
                                chatModel.append({ message: trimmedText, isUser: true })
                                chatModel.append({ message: "", isUser: false })
                                if (typeof controller !== "undefined") controller.generate(trimmedText)
                                inputField.text = ""
                                chatListView.forceActiveFocus()
                            }
                            background: Rectangle { radius: height/2; color: sendButton.enabled ? "#007AFF" : "#3a3a3c" }
                        }
                    }
                    Text {
                        text: "Disclaimer: Zippy AI can make mistakes. Double check all important info."
                        color: "#cccccc"
                        font.pointSize: sz(9)
                        Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }

    // ===== MOBILE KEYBOARD HANDLING =====
    InputPanel {
        id: inputPanel
        z: 99
        anchors.horizontalCenter: parent.horizontalCenter
        y: window.height
        width: window.width * .90
        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges { target: inputPanel; y: window.height - inputPanel.height }
        }
    }
}
