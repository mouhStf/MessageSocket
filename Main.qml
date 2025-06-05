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
    onReceivedMessage: function(message) {
      messages.append({message: message});
      messageList.positionViewAtEnd();
    }
    onSendingFile: function (name, tot, don) {
      if (don !== tot)
        sendI.text = ~~((don / tot) * 100) + "% " + name
      else
        sendI.text = ""
    }
    onReceivingFile: function (name, tot, don) {
      recvF.text = ~~((don / tot) * 100) + "% " + name
    }
    onReceivedFile: function(filePath) {
      recvF.text = "";
      var parts = filePath.toString().split("/");
      rfiles.append({
        src: filePath,
        name: decodeURI(parts[parts.length-1])
      })
    }
  }
  
  ListModel {
    id: files
  }
  ListModel {
    id: messages
  }
  ListModel {
    id: rfiles
  }

      
  Frame {
    anchors.fill: parent
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
              clip: true
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
            Label {
              id: sendI
              visible: text !== ""
            }
            RowLayout {
              ToolButton {
                text: "+ Add files"
                onClicked: fileDialog.open()
              }
              ToolButton {
                text: "Send"
                enabled: wtch.fileSocketState === 3
                onClicked: function() {
                  for (var i = 0; i < files.count; i++) {
                    wtch.sendFile(files.get(i).src);                  
                  }
                }
              }
            }
            Label {
              text: "Received files : " + rfiles.count
            }
            Label {
              id: recvF
              visible: text !== ""
            }
            ListView {
              clip: true
              Layout.fillHeight: true
              Layout.fillWidth: true
              model: rfiles
              delegate: Text {
                required property string name
                required property string src
                text: name
              }
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
              
              ToolButton {
                text: "Send"
                anchors{
                  bottom: parent.bottom
                  right: parent.right
                  margins: 5
                }
                enabled: wtch.messageSocketState === 3
                onClicked: function() {
                  wtch.sendMessage(textArea.text);
                }
              }
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
          RowLayout {
            RowLayout {
              Rectangle {
                Layout.preferredWidth: 10
                Layout.preferredHeight: 10
                radius: 5
                color: wtch.listenning ? "green" : "red"
              }
              Label {
                text: "Server"
              }
            }
            ColumnLayout {
              RowLayout {
                Rectangle {
                  Layout.preferredWidth: 10
                  Layout.preferredHeight: 10
                  radius: 5
                  color: wtch.messageSocketState === 3 ? "green" : "red"
                }
                Label {
                  text: "Message"
                }
              }
              RowLayout {
                Rectangle {
                  Layout.preferredWidth: 10
                  Layout.preferredHeight: 10
                  radius: 5
                  color: wtch.fileSocketState ? "green" : "red"
                }
                Label {
                  text: "File"
                }
              }
            }
          }
          Rectangle {
            color: "black"
            Layout.preferredWidth: 1
            Layout.fillHeight: true
          }
          TextField {
            id: host
            placeholderText: "Host"
            text: "127.0.0.1"
            Layout.preferredWidth: 110
          }
          TextField {
            id: port
            placeholderText: "Port"
            text: "8090"
            Layout.preferredWidth: 50
            validator: IntValidator {
            }
          }
          ToolButton {
            text: "Connect Message"
            enabled: !wtch.listenning
            onClicked: function() {
              wtch.connectMessageSocket(host.text, port.text);
            }
          }
          ToolButton {
            text: "Connect File"
            enabled: !wtch.listenning
            onClicked: function() {
              wtch.connectFileSocket(host.text, port.text);
            }
          }
          ToolButton {
            text: "Serv"
            onClicked: function() {
              wtch.listen(host.text, port.text);
            }
          }
          Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
          }            
        }
      }
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
          });
        }
      }
    }
  }
}
