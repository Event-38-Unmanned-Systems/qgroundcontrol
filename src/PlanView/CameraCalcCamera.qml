import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts  1.2

import QGroundControl                   1.0
import QGroundControl.ScreenTools       1.0
import QGroundControl.Controls          1.0
import QGroundControl.FactControls      1.0
import QGroundControl.Palette           1.0

// Camera calculator "Camera" section for mission item editors
ColumnLayout {
    spacing: _margin

    property var    cameraCalc
    property real   _margin:            ScreenTools.defaultFontPixelWidth / 2
    property real   _fieldWidth:        ScreenTools.defaultFontPixelWidth * 10.5
    property var    _vehicle:           QGroundControl.multiVehicleManager.activeVehicle ? QGroundControl.multiVehicleManager.activeVehicle : QGroundControl.multiVehicleManager.offlineEditingVehicle
    property var    _vehicleCameraList: _vehicle ? _vehicle.staticCameraList : []

    // mavlink camera info for changing camera selected
    property var    _mavlinkCameraManager:                      _vehicle ? _vehicle.cameraManager : null
    property int    _mavlinkCameraManagerCurCameraIndex:        _mavlinkCameraManager ? _mavlinkCameraManager.currentCamera : -1
    property bool   _noMavlinkCameras:                          _mavlinkCameraManager ? _mavlinkCameraManager.cameras.count === 0 : true
    property var    _mavlinkCamera:                             !_noMavlinkCameras ? (_mavlinkCameraManager.cameras.get(_mavlinkCameraManagerCurCameraIndex) && _mavlinkCameraManager.cameras.get(_mavlinkCameraManagerCurCameraIndex).paramComplete ? _mavlinkCameraManager.cameras.get(_mavlinkCameraManagerCurCameraIndex) : null) : null

    Component.onCompleted: {

        if (!_noMavlinkCameras && _mavlinkCamera.vendor !== "" && _mavlinkCamera.modelName !== ""){
            //exceptions for current companion computers sent out to show user friendly interfaces.

            cameraCalc.cameraBrand = _mavlinkCamera.vendor;

            if (_mavlinkCamera.modelName === "ILCE-6100"){
                cameraCalc.cameraModel = "a6000 20mm";
            }
            else if (_mavlinkCamera.modelName === "RX1RM2" ){
                cameraCalc.cameraModel = "DSC-RX1R II 35mm";
            }
            else if (_mavlinkCamera.modelName === "ILCE-7RM4A" ){
                cameraCalc.cameraModel = "Sony A7R IV 24mm";
            }
            else {
              cameraCalc.cameraModel = _mavlinkCamera.modelName
            }

            cameraBrandCombo.selectCurrentBrand();
            cameraModelCombo.selectCurrentModel();
        }
        else{
        cameraBrandCombo.selectCurrentBrand()
        cameraModelCombo.selectCurrentModel()
        }
    }

    QGCPalette { id: qgcPal; colorGroupEnabled: true }

    ColumnLayout {
        Layout.fillWidth:   true
        spacing:            _margin

        QGCComboBox {
            id:                 cameraBrandCombo
            Layout.fillWidth:   true
            model:              cameraCalc.cameraBrandList
            onModelChanged:     selectCurrentBrand()
            onActivated:        cameraCalc.cameraBrand = currentText

            Connections {
                target:                 cameraCalc
                onCameraBrandChanged:   cameraBrandCombo.selectCurrentBrand()
            }

            function selectCurrentBrand() {
                currentIndex = cameraBrandCombo.find(cameraCalc.cameraBrand)
            }
        }

        QGCComboBox {
            id:                 cameraModelCombo
            Layout.fillWidth:   true
            model:              cameraCalc.cameraModelList
            visible:            !cameraCalc.isManualCamera && !cameraCalc.isCustomCamera
            onModelChanged:     selectCurrentModel()
            onActivated:        cameraCalc.cameraModel = currentText

            Connections {
                target:                 cameraCalc
                onCameraModelChanged:   cameraModelCombo.selectCurrentModel()
            }

            function selectCurrentModel() {
                currentIndex = cameraModelCombo.find(cameraCalc.cameraModel)
            }
        }

        // Camera based grid ui
        ColumnLayout {
            Layout.fillWidth:   true
            spacing:            _margin
            visible:            !cameraCalc.isManualCamera

            RowLayout {
                Layout.alignment:   Qt.AlignHCenter
                spacing:            _margin
                visible:            !cameraCalc.fixedOrientation.value

                QGCRadioButton {
                    width:          _editFieldWidth
                    text:           "Landscape"
                    checked:        !!cameraCalc.landscape.value
                    onClicked:      cameraCalc.landscape.value = 1
                }

                QGCRadioButton {
                    id:             cameraOrientationPortrait
                    text:           "Portrait"
                    checked:        !cameraCalc.landscape.value
                    onClicked:      cameraCalc.landscape.value = 0
                }
            }

            // Custom camera specs
            ColumnLayout {
                id:                 custCameraCol
                Layout.fillWidth:   true
                spacing:            _margin
                enabled:            cameraCalc.isCustomCamera

                RowLayout {
                    Layout.fillWidth:   true
                    spacing:            _margin

                    Item { Layout.fillWidth: true }
                    QGCLabel {
                        Layout.preferredWidth:  _root._fieldWidth
                        text:                   qsTr("Width")
                    }
                    QGCLabel {
                        Layout.preferredWidth:  _root._fieldWidth
                        text:                   qsTr("Height")
                    }
                }

                RowLayout {
                    Layout.fillWidth:   true
                    spacing:            _margin

                    QGCLabel { text: qsTr("Sensor"); Layout.fillWidth: true }
                    FactTextField {
                        Layout.preferredWidth:  _root._fieldWidth
                        fact:                   cameraCalc.sensorWidth
                    }
                    FactTextField {
                        Layout.preferredWidth:  _root._fieldWidth
                        fact:                   cameraCalc.sensorHeight
                    }
                }

                RowLayout {
                    Layout.fillWidth:   true
                    spacing:            _margin

                    QGCLabel { text: qsTr("Image"); Layout.fillWidth: true }
                    FactTextField {
                        Layout.preferredWidth:  _root._fieldWidth
                        fact:                   cameraCalc.imageWidth
                    }
                    FactTextField {
                        Layout.preferredWidth:  _root._fieldWidth
                        fact:                   cameraCalc.imageHeight
                    }
                }

                RowLayout {
                    Layout.fillWidth:   true
                    spacing:            _margin
                    QGCLabel {
                        text:                   qsTr("Focal length")
                        Layout.fillWidth:       true
                    }
                    FactTextField {
                        Layout.preferredWidth:  _root._fieldWidth
                        fact:                   cameraCalc.focalLength
                    }
                }
            }
        }
    }
}
