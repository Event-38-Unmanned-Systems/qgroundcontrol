import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs  1.2
import QtQuick.Extras   1.4
import QtQuick.Layouts  1.2

import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Vehicle       1.0
import QGroundControl.Controls      1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl.Palette       1.0
import QGroundControl.FlightMap     1.0

Rectangle {
    id:         _root
    height:     childrenRect.y + childrenRect.height + _margin
    width:      availableWidth
    color:      qgcPal.windowShadeDark
    radius:     _radius

    property bool   transectAreaDefinitionComplete: true
    property string transectAreaDefinitionHelp:     _internalError
    property string transectValuesHeaderName:       _internalError
    property var    transectValuesComponent:        undefined
    property var    presetsTransectValuesComponent: undefined

    readonly property string _internalError: "Internal Error"

    property var    _missionItem:               missionItem
    property real   _margin:                    ScreenTools.defaultFontPixelWidth / 2
    property real   _fieldWidth:                ScreenTools.defaultFontPixelWidth * 10.5
    property var    _vehicle:                   QGroundControl.multiVehicleManager.activeVehicle ? QGroundControl.multiVehicleManager.activeVehicle : QGroundControl.multiVehicleManager.offlineEditingVehicle
    property real   _cameraMinTriggerInterval:  _missionItem.cameraCalc.minTriggerInterval.rawValue
    property string _doneAdjusting:             qsTr("Done")
    property bool   _presetsAvailable:          _missionItem.presetNames.length !== 0

    function polygonCaptureStarted() {
        _missionItem.clearPolygon()
    }

    function polygonCaptureFinished(coordinates) {
        for (var i=0; i<coordinates.length; i++) {
            _missionItem.addPolygonCoordinate(coordinates[i])
        }
    }

    function polygonAdjustVertex(vertexIndex, vertexCoordinate) {
        _missionItem.adjustPolygonCoordinate(vertexIndex, vertexCoordinate)
    }

    function polygonAdjustStarted() { }
    function polygonAdjustFinished() { }

    QGCPalette { id: qgcPal; colorGroupEnabled: true }

    ColumnLayout {
        id:                 editorColumn
        anchors.margins:    _margin
        anchors.top:        parent.top
        anchors.left:       parent.left
        anchors.right:      parent.right

        QGCLabel {
            id:                     transectAreaDefinitionCompleteLabel
            Layout.fillWidth:       true
            wrapMode:               Text.WordWrap
            horizontalAlignment:    Text.AlignHCenter
            text:                   transectAreaDefinitionHelp
            visible:                !transectAreaDefinitionComplete || _missionItem.wizardMode
        }

        ColumnLayout {
            Layout.fillWidth:   true
            spacing:            _margin
            visible:            transectAreaDefinitionComplete && !_missionItem.wizardMode

            TransectStyleComplexItemTabBar {
                id:                 tabBar
                Layout.fillWidth:   true
            }

            // Grid tab
            ColumnLayout {
                Layout.fillWidth:   true
                spacing:            _margin
                visible:            tabBar.currentIndex === 0

                SectionHeader {
                    id:                 statsHeader
                    Layout.fillWidth:   true
                    text:               qsTr("Stats")
                }

                TransectStyleComplexItemStats {
                    Layout.fillWidth:   true
                    visible:            statsHeader.checked
                }

                QGCLabel {
                    Layout.fillWidth:   true
                    text:               qsTr("WARNING: Photo interval is below minimum interval (%1 secs) supported by camera.").arg(_cameraMinTriggerInterval.toFixed(1))
                    wrapMode:           Text.WordWrap
                    color:              qgcPal.warningText
                    visible:            _missionItem.cameraShots > 0 && _cameraMinTriggerInterval !== 0 && _cameraMinTriggerInterval > _missionItem.timeBetweenShots
                }

                SectionHeader {
                    id:                 cameraHeader
                    Layout.fillWidth:   true
                    text:               "overlap and altitude"
                }

                CameraCalcGrid {
                    visible:                        cameraHeader.checked
                    Layout.fillWidth:               true
                    cameraCalc:                     _missionItem.cameraCalc
                    vehicleFlightIsFrontal:         true
                    distanceToSurfaceLabel:         qsTr("Altitude")
                    frontalDistanceLabel:           qsTr("Trigger Dist")
                    sideDistanceLabel:              qsTr("Spacing")
                }

                SectionHeader {
                    id:                 transectValuesHeader
                    Layout.fillWidth:   true
                    text:               "flight path"
                }

                Loader {
                    Layout.fillWidth:   true
                    visible:            transectValuesHeader.checked
                    sourceComponent:    transectValuesComponent

                    property bool forPresets: false
                }

                QGCButton {
                    Layout.alignment:   Qt.AlignHCenter
                    text:               qsTr("Rotate Entry Point")
                    onClicked:          _missionItem.rotateEntryPoint()
                    visible:            transectValuesHeader.checked
                }

            } // Grid Column

            // Camera Tab
            CameraCalcCamera {
                Layout.fillWidth:   true
                visible:            tabBar.currentIndex === 1
                cameraCalc:         _missionItem.cameraCalc
            }

            // Terrain Tab
            TransectStyleComplexItemTerrainFollow {
                Layout.fillWidth:   true
                spacing:            _margin
                visible:            tabBar.currentIndex === 2
                missionItem:        _missionItem
            }

            // Presets Tab

        } // Top level  Column

        Component {
            id: savePresetDialog

            QGCPopupDialog {
                id:         popupDialog
                title:      qsTr("Save Preset")
                buttons:    StandardButton.Save | StandardButton.Cancel

                function accept() {
                    if (presetNameField.text != "") {
                        _missionItem.savePreset(presetNameField.text.trim())
                        hideDialog()
                    }
                }

                ColumnLayout {
                    width:      ScreenTools.defaultFontPixelWidth * 30
                    spacing:    ScreenTools.defaultFontPixelHeight

                    QGCLabel {
                        Layout.fillWidth:   true
                        text:               qsTr("Save the current settings as a named preset.")
                        wrapMode:           Text.WordWrap
                    }

                    QGCLabel {
                        text: qsTr("Preset Name")
                    }

                    QGCTextField {
                        id:                 presetNameField
                        Layout.fillWidth:   true
                        placeholderText:    qsTr("Enter preset name")

                        Component.onCompleted:  validateText(presetNameField.text)
                        onTextChanged:          validateText(text)

                        function validateText(text) {
                            if (text.trim() === "") {
                                nameError.text = qsTr("Preset name cannot be blank.")
                                popupDialog.disableAcceptButton()
                            } else if (text.includes("/")) {
                                nameError.text = qsTr("Preset name cannot include the \"/\" character.")
                                popupDialog.disableAcceptButton()
                            } else {
                                nameError.text = ""
                                popupDialog.enableAcceptButton()
                            }
                        }
                    }

                    QGCLabel {
                        id:                 nameError
                        Layout.fillWidth:   true
                        wrapMode:           Text.WordWrap
                        color:              QGroundControl.globalPalette.warningText
                        visible:            text !== ""
                    }
                }
            }
        }
    }
}
