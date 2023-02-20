/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick                      2.11
import QtQuick.Controls             2.4
import QtQml.Models                 2.1

import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Controls      1.0
import QGroundControl.FlightDisplay 1.0
import QGroundControl.Vehicle       1.0



Item {
    property var _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property var model: listModel

    // mavlink camera info for changing camera selected
    property var    _mavlinkCameraManager:                      _activeVehicle ? _activeVehicle.cameraManager : null
    property int    _mavlinkCameraManagerCurCameraIndex:        _mavlinkCameraManager ? _mavlinkCameraManager.currentCamera : -1
    property bool   _noMavlinkCameras:                          _mavlinkCameraManager ? _mavlinkCameraManager.cameras.count === 0 : true
    property var    _mavlinkCamera:                             !_noMavlinkCameras ? (_mavlinkCameraManager.cameras.get(_mavlinkCameraManagerCurCameraIndex) && _mavlinkCameraManager.cameras.get(_mavlinkCameraManagerCurCameraIndex).paramComplete ? _mavlinkCameraManager.cameras.get(_mavlinkCameraManagerCurCameraIndex) : null) : null
    property var    _supportedCamera:                           !_noMavlinkCameras ? _mavlinkCamera.vendor !== "" && _mavlinkCamera.modelName !== "" : false
    property bool   capSuccess: false
    Timer {
        id: calibrationTimer
    }

    Timer {
        id: cameraTimer
    }
    function delay2(delayTime,cb)
    {
        cameraTimer.interval = delayTime;
        cameraTimer.repeat = false;
        cameraTimer.triggered.connect(cb);
        cameraTimer.start();
    }

    function delay(delayTime,cb)
    {
        calibrationTimer.interval = delayTime;
        calibrationTimer.repeat = false;
        calibrationTimer.triggered.connect(cb);
        calibrationTimer.start();
    }

    PreFlightCheckModel {
        id:     listModel

        PreFlightCheckGroup {
            name: qsTr("Hardware Checks")

            PreFlightCheckButton {
                name:           qsTr("Hardware")
                manualText:     qsTr("Props mounted? Wings secured? Tail secured?")
            }

            PreFlightBatteryCheck {
                failurePercent:                 80
                allowFailurePercentOverride:    false
            }

            PreFlightCheckButton {
                name:           qsTr("Tablet Battery")
                manualText:     qsTr("Tablet Battery above 60%?")
            }

            PreFlightSensorsHealthCheck {
            }

            PreFlightGPSCheck {
                failureSatCount:        9
                allowOverrideSatCount:  true
            }
        }

        PreFlightCheckGroup {
            name: qsTr("Pre-launch Checks")

            PreFlightCheckButton {
                height:         10 * ScreenTools.defaultFontPixelWidth
                name:            qsTr("Heading")
                manualText:      qsTr("Icon heading matches actual heading?")
                image: qsTr("/qmlimages/PaperPlane.svg")
                }
            PreFlightCheckButton {
                name:        qsTr("Mission")
                manualText:  qsTr("Please confirm mission is valid (waypoints valid, no terrain collision).")
            }
            PreFlightCheckButton {
                name:            qsTr("Actuators")
                id: actuators

                manualText:     {
                    if (_activeVehicle.joystickEnabled || QGroundControl.settingsManager.appSettings.virtualJoystick.rawValue){
                             manualText = qsTr("Verify control surfaces are smooth with the joystick");
                       }

                    else {
                            manualText = "Press to move control surfaces. Did they work properly?"}
                         }

                onPressed: {
                    if (_manualState != _statePassed){
                        if (_activeVehicle.joystickEnabled || QGroundControl.settingsManager.appSettings.virtualJoystick.rawValue){
                        }
                               else {
                                    _activeVehicle.preflightServoTest(1,-1);
                                    delay(3000,function() {
                                        _activeVehicle.preflightServoTest(-1,1);
                                    })
                                }
                            }
                        }
                    }
            PreFlightSoundCheck {
            }
        }

        PreFlightCheckGroup {
            name: qsTr("Last preparations before launch")

            PreFlightCheckButton {
                name:        qsTr("Calibrate Airspeed")
                manualText:  qsTr("Start calibration")
                onPressed: if (_manualState != _statePassed){_activeVehicle.preflightCalibration()}
            }
            // Check list item group 2 - Final checks before launch
            PreFlightCheckButton {
                visible: _activeVehicle ? (!_activeVehicle.gimbalData) : true
                name:        {
                    if (_supportedCamera){
                        //exceptions for current companion computers sent out to show user friendly interfaces.

                        if (_mavlinkCamera.modelName === "ILCE-6100"){
                            name = "Sony A6000";
                        }
                        else if (_mavlinkCamera.modelName === "RX1RM2" ){
                            name = "Sony RX1R II";
                        }
                        else if (_mavlinkCamera.modelName === "ILCE-7RM4A" ){
                            name = "Sony A7R IV";
                        }
                        else {
                          name = _mavlinkCamera.modelName
                        }
                    }
                    //set to passed and hide if nighthawk
                    else if (_activeVehicle.gimbalData){
                        name = "NightHawk"
                        passed = _statePassed
                        _manualstate = _statePassed
                    }
                    else {name = "Payload"}
                }
                manualText:  {
                    if (_supportedCamera){
                                    if (_mavlinkCamera.modelName === "RX1RM2") {
                                        manualText = "Lens cap removed? SD card inserted into camera?"
                                        }
                                    else{ manualText = "Lens cap removed?"}
                        }
                    else{ manualText = "Lens cap removed?"}
                }
            }

            PreFlightCheckButton {
                name:   {
                    if (_supportedCamera){
                        //exceptions for current companion computers sent out to show user friendly interfaces.

                        if (_mavlinkCamera.modelName === "ILCE-6100"){
                            name = "Sony A6000";
                        }
                        else if (_mavlinkCamera.modelName === "RX1RM2" ){
                            name = "Sony RX1R II";
                        }
                        else if (_mavlinkCamera.modelName === "ILCE-7RM4A" ){
                            name = "Sony A7R IV";
                        }
                        else {
                          name = _mavlinkCamera.modelName
                        }
                    }
                    else if (_activeVehicle.gimbalData){
                        name = "NightHawk"
                    }
                    else {name = "Payload"}
                }

                manualText:  { if (_supportedCamera){
                        if (_mavlinkCamera.modelName === "RX1RM2" ){
                            manualText = ("Press to begin payload test. Listen for image capture.");
                        }
                        else{
                        manualText = ("Press to begin payload test");
                        }
                    }
                    else if (_activeVehicle.gimbalData){
                        manualText = "Verify manual contol. Press to stow for takeoff."
                    }
                    else { manualText = ("Press to capture image. Image captured?"); }
                }
                onPressed:   { if (_manualState != _statePassed){

                        if (_supportedCamera){

                            telemetryFailure = false

                            if (_mavlinkCamera.storageStatus === QGCCameraControl.STORAGE_READY){

                                var curcapcount = _activeVehicle.imagesCaptures;

                                _mavlinkCamera.takePhoto()

                                _telemetryState = _statePending
                                telemetryTextFailure = "Tests Running";

                                delay2(4000,function() {
                                if(curcapcount === _activeVehicle.imagesCaptures){
                                    telemetryFailure = true
                                    telemetryTextFailure = "Capture failed Check Camera";
                                }
                                else{
                                    _telemetryState = _statePassed
                                    telemetryFailure = false
                                }
                            })

                            }

                            else {
                                telemetryFailure = true
                                telemetryTextFailure = ("Storage: Not ready. Tests aborted.");
                            }
                        }
                        else if (_activeVehicle.gimbalData){
                            _activeVehicle.nighthawksetMode(0);
                        }
                        else { _activeVehicle.triggerSimpleCamera()}
             }
                    if (_manualState == _statePassed){ telemetryFailure = false;}
           }

        }
            PreFlightCheckButton {
                name:        "Wind & weather"
                manualText:  qsTr("Wind within limits? Lauching into the wind?")
            }

            PreFlightCheckButton {
                name:        qsTr("Flight area")
                manualText:  qsTr("Launch area and path free of obstacles/people?")
            }
        }
    }
}

