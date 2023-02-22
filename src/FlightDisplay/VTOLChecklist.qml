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
        id: preflightCalibrationTimer
        property var myCB
        property int timesToRun: 5
        property int timesRun: 0
        property int delayTime: 1000
        repeat: true
    }

    Timer {
        id: actuatorTimer
        property var myCB
        property int timesToRun: 1
        property int timesRun: 0
        property int delayTime: 3000
        repeat: false
    }

    Timer {
        id: cameraTimer
        property var myCB
        property int timesToRun: 10
        property int timesRun: 0
        property int delayTime: 1000
        repeat: true
    }


    function delay(cb,timer)
    {
        if (timer.myCB) timer.triggered.disconnect(timer.myCB)
        timer.myCB = cb
        timer.interval = timer.delayTime;
        timer.triggered.connect(timer.myCB);
        timer.start();
    }

    PreFlightCheckModel {
        id:     listModel

       /* PreFlightCheckGroup {
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
        } */

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
                                    delay(function() {
                                        _activeVehicle.preflightServoTest(-1,1);
                                    },actuatorTimer)
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
                onPressed: if (_manualState != _statePassed){
                                   //reset calibration timeout settings
                                   preflightCalibrationTimer.timesRun = 0
                                   preflightCalibrationTimer.repeat = true;
                                   //reset calibration timeout UI overrides
                                   calibrationOverride = false
                                   telemetryFailure = false
                                   //Trying to attempt a calibration. Set to incomplete until feedback received
                                   _activeVehicle.airspeedCalibrated = false
                                   //call preflight calibration for vehicle
                                   _activeVehicle.preflightCalibration()

                                   _telemetryState = _statePending
                                   telemetryTextFailure = "Calibrating";

                                   delay(function() {

                                   if(_activeVehicle.airspeedCalibrated){
                                       _telemetryState = _statePassed
                                       telemetryFailure = false
                                       preflightCalibrationTimer.repeat = false;
                                   }
                                   else if (preflightCalibrationTimer.timesRun >= preflightCalibrationTimer.timesToRun) {
                                    telemetryFailure = true
                                    _telemetryState = _stateFailed
                                    telemetryTextFailure = "Calibration failed. Try again.";
                                    preflightCalibrationTimer.repeat = false;
                                   }
                                   preflightCalibrationTimer.timesRun = preflightCalibrationTimer.timesRun + 1
                               }, preflightCalibrationTimer)

                               }
                               else if (_manualState == _statePassed){
                               calibrationOverride = true
                               _manualState = _statePending
                               telemetryFailure = false
                           }
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

                        calibrationOverride = false
                        telemetryFailure = false

                        if (_supportedCamera){
                            //reset calibration timeout settings
                            cameraTimer.timesRun = 0
                            cameraTimer.repeat = true;

                            if (_mavlinkCamera.storageStatus === QGCCameraControl.STORAGE_READY){

                                var curcapcount = _activeVehicle.imagesCaptures;

                                _mavlinkCamera.takePhoto()

                                _telemetryState = _statePending
                                telemetryTextFailure = "Tests Running";

                                delay(function() {
                                if(cameraTimer.timesRun >= cameraTimer.timesToRun && curcapcount === _activeVehicle.imagesCaptures){
                                    telemetryFailure = true
                                    telemetryTextFailure = "Capture failed Check Camera";
                                    cameraTimer.repeat = false
                                }
                                else if (curcapcount < _activeVehicle.imagesCaptures){
                                    _telemetryState = _statePassed
                                    telemetryFailure = false
                                    cameraTimer.repeat = false
                                }
                                cameraTimer.timesRun = cameraTimer.timesRun + 1
                            },cameraTimer)

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
                    if (_manualState == _statePassed){
                        calibrationOverride = true
                        _manualState = _statePending
                        telemetryFailure = false}
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

