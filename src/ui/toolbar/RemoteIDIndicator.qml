/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts  1.2

import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl                       1.0
import QGroundControl.Controls              1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.Palette               1.0

//-------------------------------------------------------------------------
//-- Message Indicator
Item {
    id:             _root
    width:          height
    anchors.top:    parent.top
    anchors.bottom: parent.bottom

    visible: QGroundControl.multiVehicleManager.RIDEnabled

    property bool emergencyDeclared: false

    property var    _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle

    QGCColoredImage {
        anchors.fill:       parent
        source:             "/res/remote-id.svg"
        sourceSize.height:  height
        fillMode:           Image.PreserveAspectFit
    }

    MouseArea {
        anchors.fill:   parent
        onClicked:      mainWindow.showIndicatorPopup(_root, remoteIDStatus)
    }

    Component {

        id: remoteIDStatus

            Rectangle {
                width:          mainLayout.width   + mainLayout.anchors.margins * 2
                height:         mainLayout.height  + mainLayout.anchors.margins * 2
                radius:         ScreenTools.defaultFontPixelHeight / 2
                color:          qgcPal.window
                border.color:   qgcPal.text

                MouseArea {
                    id: remoteIDStatusMouseArea
                    anchors.fill:       parent

                    onDoubleClicked:    {_activeVehicle.ridSetEmergency(); emergencyDeclared = true;}

                }


                ColumnLayout {
                    id:                 mainLayout
                    anchors.margins:    ScreenTools.defaultFontPixelWidth
                    anchors.top:        parent.top
                    anchors.right:      parent.right
                    spacing:            ScreenTools.defaultFontPixelHeight

                    QGCLabel {
                        Layout.alignment:   Qt.AlignCenter
                        text:               qsTr("Remote ID.")
                        font.family:        ScreenTools.demiboldFontFamily
                    }
                    QGCLabel {
                        Layout.alignment:   Qt.AlignCenter
                        text:               qsTr("Double Press to declare emergency.")
                        font.family:        ScreenTools.demiboldFontFamily
                    }

                    RowLayout {
                        spacing: ScreenTools.defaultFontPixelWidth

                        ColumnLayout {


                                ColumnLayout {
                                    spacing: 0

                                    QGCLabel { text: qsTr("Remote ID Status:") }
                                    QGCLabel { text: qsTr("Emergency State:");
                                               visible: emergencyDeclared}

                                }
                        }

                        ColumnLayout {
                                ColumnLayout {
                                    spacing: 0
                                    QGCLabel { text: _activeVehicle.droneIDState === "" ? "Ready" : _activeVehicle.droneIDState }
                                    QGCLabel { text: "Pilot Emergency"
                                               visible: emergencyDeclared}
                                  }
                                }
                        }
                    }
                }
            }
    }

