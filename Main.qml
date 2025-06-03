pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import imps

ApplicationWindow {
  width: 1100
  height: 650
  visible: true
  title: "MessageSocket"
  id: window

  Watcher {
    id: wtch
    onGotMessage: function(message, fileNames) {
      console.log("Received", message, fileNames);
      var fns = "";
      if (fileNames.length > 0) {
        fns = " -- files: [";
        for (var i = 0; i < fileNames.length; i++)
          fns += fileNames[i] + "-";
        fns += "]";
      }
      messages.append({message: message + fns});
      messageList.positionViewAtEnd();
    }
  }

  StackLayout {
    anchors.fill: parent
    currentIndex: bar.currentIndex

    ListModel {
      id: files
    }
    ListModel {
      id: messages
    }
    
    Frame {
      ColumnLayout {
        anchors.fill: parent
        RowLayout {
          Frame {
            clip: true
            Layout.fillHeight: true
            Layout.preferredWidth: 150
            ColumnLayout {
              anchors.fill: parent
              Label {
                text: "Choosed files : " + files.count
              }

              ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                model: files
                delegate: Item {
                  id: it
                  required property url src;
                  required property string name;
                  required property int index;
                  height: lay.height
                  RowLayout {
                    id: lay
                    Label {
                      text: "X"
                      MouseArea {
                        anchors.fill: parent
                        onClicked: function() {
                          files.remove(it.index);
                        }
                      }
                    }
                    Text {                      
                      text: it.name
                    }
                  }
                }
              }
              
              Button {
                text: "+ Add files"
                onClicked: fileDialog.open()
              }
            }
          }
          Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
              anchors.fill: parent

              TextArea {
                id: textArea
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height / 2
              }
              Frame {
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height / 2
                ColumnLayout {
                  anchors.fill: parent
                  Label {
                    id: mess
                    text: "Received messages :"
                  }
                  ListView {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    id: messageList
                    clip: true
                    model: messages
                    delegate: Text {
                      required property string message;
                      wrapMode: Text.Wrap
                      
                      text: "-> " + message
                    }
                    ScrollBar.vertical: ScrollBar { }
                  }
                }
              }
            }
          }
        }
        ToolBar {
          Layout.fillWidth: true
          RowLayout {
            anchors.fill: parent
            Rectangle {
              Layout.preferredWidth: 10
              Layout.preferredHeight: 10
              radius: 5
              color: wtch.connected ? "green" : "red"
            }
            Label {
              text: (wtch.server ? "Server - " : "") + (wtch.connected ? "Connected" : "Diconnected")
            }
            Rectangle {
              color: "black"
              Layout.preferredWidth: 1
              Layout.fillHeight: true
            }
            ToolButton {
              text: "Connect"
              enabled: !wtch.connected && !wtch.server
              onClicked: function() {
                wtch.connectToSrv();
              }
            }
            Item {
              Layout.fillWidth: true
              Layout.fillHeight: true
            }
            ToolButton {
              text: "Send"
              enabled: wtch.connected
              onClicked: function() {
                var urls = [];
                for (var i = 0; i < files.count; i++)
                  urls.push(files.get(i).src);                
                wtch.sendMessage(textArea.text, urls);
              }
            }
          }
        }
      }
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
      for (var i = 0; i < selectedFiles.length; i++) {
        var parts = selectedFiles[i].toString().split("/");
        var there = false;
        for (var j = 0; j < files.count; j++) {
          if (files.get(j).src == selectedFiles[i]) {
            there = true;
            break;
          }
        }
        if (!there) {
          files.append({
            src: selectedFiles[i],
            name: parts[parts.length-1]
          })
        }
      }
    }
  }
}
