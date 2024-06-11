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
import QGroundControl.SettingsManager       1.0

//-------------------------------------------------------------------------
//-- Message Indicator



Item {
    id:             _root
    width:          height
    anchors.top:    parent.top
    anchors.bottom: parent.bottom

    property var    _videoSettings:             QGroundControl.settingsManager.videoSettings
    property var    _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property real _margins:         ScreenTools.defaultFontPixelHeight

    Image {
        anchors.fill:       parent
        source:             "/res/NightHawk.png"
        sourceSize.height:  height
        fillMode:           Image.PreserveAspectCrop
    }

    MouseArea {
        anchors.fill:   parent
        onClicked:      mainWindow.showIndicatorPopup(_root, nextVisionUI)
    }

    Component {

        id: nextVisionUI

            Rectangle {
                width:          mainLayout.width   + mainLayout.anchors.margins * 4
                height:         mainLayout.height  + mainLayout.anchors.margins * 4
                radius:         ScreenTools.defaultFontPixelHeight / 2
                color:          qgcPal.window
                border.color:   qgcPal.text


                ColumnLayout {
                    id:                 mainLayout
                    anchors.margins:    ScreenTools.defaultFontPixelWidth
                    anchors.top:        parent.top
                    anchors.right:      parent.right
                    spacing:            ScreenTools.defaultFontPixelHeight

                    Column {

                            Layout.alignment: Qt.AlignHCenter

                                        spacing: _margins

                                        GridLayout {

                                            id:             gridlayout
                                            columnSpacing:  _margins
                                            rowSpacing:     _margins
                                            columns:        2
                                            QGCLabel { text: qsTr("Camera Pallet:")
                                                                   font.bold: true}
                                                        FactComboBox {
                                                            fact:                   QGroundControl.settingsManager.appSettings.nextVisionModes
                                                            indexModel:         false
                                                            Layout.fillWidth:   true
                                                            onActivated: {
                                                                _activeVehicle.nightHawksetPallet(currentIndex);
                                                            }
                                                        }

                                            QGCLabel { text: qsTr("FCC Calibration:")
                                                       font.bold: true}

                                            Image {
                                                height:                 35
                                                width:                  35
                                                source: "/InstrumentValueIcons/refresh.svg"
                                                MouseArea {
                                                    anchors.fill: parent;
                                                    onClicked: {_activeVehicle.nightHawkfccCalibration();}
                                                }
                                                fillMode:               Image.PreserveAspectFit
                                                sourceSize.height:      height
                                                Layout.alignment:       Qt.AlignVCenter | Qt.AlignHCenter
                                            }
                                        } // GridLayout
                                    } // Column
                    }


                }
            }
    }

