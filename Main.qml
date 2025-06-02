import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
  width: 1100
  height: 650
  visible: true
  title: "MessageSocket"
  id: window

  StackLayout {
    anchors.fill: parent
    currentIndex: bar.currentIndex
    Pane {
      ColumnLayout {
        anchors.fill: parent
        TextArea {
          id: textArea
          Layout.fillWidth: true
          Layout.fillHeight: true
        }
        ToolBar {
          Layout.fillWidth: true
          RowLayout {
            anchors.fill: parent
            ToolButton {
              text: "Connect"
              onClicked: function() {
                socket.doConnect("127.0.0.1", "8094")
              }
            }
            Label {
              text: "Files -- " + fileDialog.selectedFiles.length
            }
            ToolButton {
              text: "Choose files"
              onClicked: fileDialog.open()
            }
            ToolButton {
              text: "Send"
              onClicked: function() {
                socket.sendMessage(textArea.text, fileDialog.selectedFiles)
              }
            }
          }
        }
      }
    }
    Pane {
    }
  }

  footer: TabBar {
    id: bar
    TabButton {
      text: "Envoi"
    }
    TabButton {
      text: "Reception"
    }
  }

  FileDialog {
    id: fileDialog
    title: "Please choose files"
    fileMode: FileDialog.OpenFiles
    
    onAccepted: function() {      
      console.log("Selected files:", selectedFiles)
    }
  }
}
