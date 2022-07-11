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
import QtQuick.Dialogs  1.2
import QtQuick.Layouts  1.2

import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Vehicle       1.0
import QGroundControl.Controls      1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl.Palette       1.0

// Editor for Fixed Wing Landing Pattern complex mission item
Rectangle {
    id:         _root
    height:     visible ? ((editorColumn.visible ? editorColumn.height : editorColumnNeedTakeoffPoint.height) + (_margin * 2)) : 0
    width:      availableWidth
    color:      qgcPal.windowShadeDark
    radius:     _radius

    // The following properties must be available up the hierarchy chain
    //property real   availableWidth    ///< Width for control
    //property var    missionItem       ///< Mission Item for editor

    property var    _masterControler:           masterController
    property var    _missionController:         _masterControler.missionController
    property var    _missionVehicle:            _masterControler.controllerVehicle
    property real   _margin:                    ScreenTools.defaultFontPixelWidth / 2
    property real   _spacer:                    ScreenTools.defaultFontPixelWidth / 2
    property string _setToVehicleHeadingStr:    qsTr("Set to vehicle heading")
    property string _setToVehicleLocationStr:   qsTr("Set to vehicle location")
    property bool   _showCameraSection:         !_missionVehicle.apmFirmware
    property int    _altitudeMode:              missionItem.altitudesAreRelative ? QGroundControl.AltitudeModeRelative : QGroundControl.AltitudeModeAbsolute


    Column {
        id:                 editorColumn
        anchors.margins:    _margin
        anchors.left:       parent.left
        anchors.right:      parent.right
        spacing:            _margin
        visible:            !editorColumnNeedTakeoffPoint.visible

        SectionHeader {
            id:             vtolTakeoffSection
            anchors.left:   parent.left
            anchors.right:  parent.right
            text:           qsTr("VTOL Takeoff")
        }

        Column {
            anchors.left:       parent.left
            anchors.right:      parent.right
            spacing:            _margin
            visible:            vtolTakeoffSection.checked

            Item { width: 1; height: _spacer }

            GridLayout {
                anchors.left:    parent.left
                anchors.right:   parent.right
                columns:         2

                QGCLabel { text: qsTr("Altitude") }

                AltitudeFactTextField {
                    Layout.fillWidth:   true
                    fact:               missionItem.vtolAlt
                    altitudeMode:       _altitudeMode
                }
            }
        }

        SectionHeader {
            id:             climboutPointSection
            anchors.left:   parent.left
            anchors.right:  parent.right
            text:           qsTr("Climbout point")
        }


        Column {
            anchors.left:       parent.left
            anchors.right:      parent.right
            spacing:            _margin
            visible:            climboutPointSection.checked

            Item { width: 1; height: _spacer }

            GridLayout {
                anchors.left:    parent.left
                anchors.right:   parent.right
                columns:         2

                QGCLabel { text: qsTr("Altitude") }

                AltitudeFactTextField {
                    Layout.fillWidth:   true
                    fact:               missionItem.climboutAlt
                    altitudeMode:       _altitudeMode
                }

                QGCLabel { text: qsTr("Takeoff Dist") }

                FactTextField {
                    fact:               missionItem.takeoffDist
                    Layout.fillWidth:   true
                }

            }
        }

        Item { width: 1; height: _spacer }
 Column {
        FactCheckBox {
            text:       qsTr("Use loiter to altitude")
            fact:       missionItem.useLoiterToAlt
        }
        Item { width: 1; height: _spacer }
        QGCCheckBox {
            anchors.right:  parent.right
            text:           qsTr("Altitudes relative to launch")
            checked:        missionItem.altitudesAreRelative
            visible:        QGroundControl.corePlugin.options.showMissionAbsoluteAltitude || !missionItem.altitudesAreRelative
            onClicked:      missionItem.altitudesAreRelative = checked
        }        
        }

        }

    Column {
        id:                 editorColumnNeedTakeoffPoint
        anchors.margins:    _margin
        anchors.top:        parent.top
        anchors.left:       parent.left
        anchors.right:      parent.right
        visible:            !missionItem.takeoffCoordSet || missionItem.wizardMode
        spacing:            ScreenTools.defaultFontPixelHeight

        Column {
            id:             takeoffCoordColumn
            anchors.left:   parent.left
            anchors.right:  parent.right
            spacing:        ScreenTools.defaultFontPixelHeight
            visible:        !missionItem.takeoffCoordSet

            QGCLabel {
                anchors.left:           parent.left
                anchors.right:          parent.right
                wrapMode:               Text.WordWrap
                horizontalAlignment:    Text.AlignHCenter
                text:                   qsTr("Click in map to set Climbout point.")
            }
        }

        ColumnLayout {
            anchors.left:   parent.left
            anchors.right:  parent.right
            spacing:        ScreenTools.defaultFontPixelHeight
            visible:        !takeoffCoordColumn.visible

            onVisibleChanged: {
                if (visible) {
                    console.log(missionItem.takeoffDist.rawValue)
                }
            }

            QGCLabel {
                Layout.fillWidth:   true
                wrapMode:           Text.WordWrap
                text:               qsTr("Drag the climbout point to adjust for wind and obstacles.")
            }

            QGCButton {
                text:               qsTr("Done")
                Layout.fillWidth:   true
                onClicked: {
                    missionItem.wizardMode = false
                    missionItem.landingDragAngleOnly = false
                    missionItem.useLoiterToAlt.rawValue = false
                }
            }
        }
    }
}
