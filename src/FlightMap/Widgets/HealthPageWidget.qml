/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.3
import QtQuick.Layouts  1.2
import QtQuick.Controls         1.2
import QtQuick.Controls.Styles  1.4
import QtQuick.Dialogs          1.2

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.Controllers   1.0
import QGroundControl.ScreenTools   1.0

/// Health page for Instrument Panel PageWidget
Column {
    width: pageWidth
    property var    _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property bool showSettingsIcon: false
    property var vehicle: QGroundControl.multiVehicleManager.activeVehicle;
    property var _unhealthySensors: QGroundControl.multiVehicleManager.activeVehicle ? QGroundControl.multiVehicleManager.activeVehicle.unhealthySensors : [ ]

    Component {
        id: calibratePressureDialogComponent

        QGCViewDialog {
            id: calibratePressureDialog

            function accept() {
                _activeVehicle.preflightCalibration()
                calibratePressureDialog.hideDialog()
            }

            QGCLabel {
                anchors.left:   parent.left
                anchors.right:  parent.right
                wrapMode:       Text.WordWrap
                text:           _helpText

                readonly property string _helpText:     qsTr("Pressure calibration will set the altitude to zero at the current pressure reading. %1").arg(_helpTextFW)
                readonly property string _helpTextFW:   qsTr("To calibrate the airspeed sensor shield it from the wind. Do not touch the sensor or obstruct any holes during the calibration.")
            }
        } // QGCViewDialog
    }

    QGCButton {
        anchors.left:   parent.left
        anchors.right:  parent.right
        text:       "Preflight Calibration"
        onClicked:  showDialog(calibratePressureDialogComponent, "Perform Preflight Calibration?", qgcView.showDialogDefaultWidth, StandardButton.Cancel | StandardButton.Ok)

        readonly property string _calibratePressureText:  qsTr("Preflight Calibration")
    }

    QGCLabel {
        width:                  parent.width
        horizontalAlignment:    Text.AlignHCenter
        text:                   qsTr("All systems healthy")
        visible:                healthRepeater.count == 0
    }

    Repeater {
        id:     healthRepeater
        model:  _unhealthySensors

        Row {
            Image {
                source:             "/qmlimages/Yield.svg"
                height:             ScreenTools.defaultFontPixelHeight
                sourceSize.height:  height
                fillMode:           Image.PreserveAspectFit
            }

            QGCLabel {
                text:   modelData
            }
        }
    }
}
