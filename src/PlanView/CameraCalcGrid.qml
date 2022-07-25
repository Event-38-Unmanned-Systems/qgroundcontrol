import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts  1.2

import QGroundControl                   1.0
import QGroundControl.ScreenTools       1.0
import QGroundControl.Controls          1.0
import QGroundControl.FactControls      1.0
import QGroundControl.Palette           1.0

// Camera calculator "Grid" section for mission item editors
Column {
    spacing: _margin

    property var    cameraCalc
    property bool   vehicleFlightIsFrontal:         true
    property string distanceToSurfaceLabel
    property string frontalDistanceLabel
    property string sideDistanceLabel

    property real   _margin:            ScreenTools.defaultFontPixelWidth / 2
    property real   _fieldWidth:        ScreenTools.defaultFontPixelWidth * 10.5
    property var    _cameraList:        [ ]
    property var    _vehicle:           QGroundControl.multiVehicleManager.activeVehicle ? QGroundControl.multiVehicleManager.activeVehicle : QGroundControl.multiVehicleManager.offlineEditingVehicle
    property var    _vehicleCameraList: _vehicle ? _vehicle.staticCameraList : []
    property bool   _cameraComboFilled: false

    readonly property int _gridTypeManual:          0
    readonly property int _gridTypeCustomCamera:    1
    readonly property int _gridTypeCamera:          2

    QGCPalette { id: qgcPal; colorGroupEnabled: true }

    Column {
        anchors.left:   parent.left
        anchors.right:  parent.right
        spacing:        _margin
        visible:        !cameraCalc.isManualCamera

        GridLayout {
            Layout.fillWidth:   true
            columnSpacing:      _margin
            rowSpacing:         _margin
            columns:            2

        QGCLabel { text: qsTr("Overlap") }
        FactTextField {
            fact:                   cameraCalc.frontalOverlap
        }
        QGCLabel { text: qsTr("Sidelap") }
        FactTextField {
            fact:                   cameraCalc.sideOverlap
        }
        QGCLabel { text: qsTr("Altitude") }
        AltitudeFactTextField {
            fact:                       cameraCalc.distanceToSurface
            altitudeMode:               cameraCalc.distanceMode
        }
        }
    } // Column - Camera spec based ui

    // No camera spec ui
    GridLayout {
        anchors.left:   parent.left
        anchors.right:  parent.right
        columnSpacing:  _margin
        rowSpacing:     _margin
        columns:        2
        visible:        cameraCalc.isManualCamera

        QGCLabel { text: distanceToSurfaceLabel }
        AltitudeFactTextField {
            fact:                       cameraCalc.distanceToSurface
            altitudeMode:               cameraCalc.distanceMode
            Layout.fillWidth:           true
        }

        QGCLabel { text: frontalDistanceLabel }
        FactTextField {
            Layout.fillWidth:   true
            fact:               cameraCalc.adjustedFootprintFrontal
        }

        QGCLabel { text: sideDistanceLabel }
        FactTextField {
            Layout.fillWidth:   true
            fact:               cameraCalc.adjustedFootprintSide
        }
    } // GridLayout
} // Column
