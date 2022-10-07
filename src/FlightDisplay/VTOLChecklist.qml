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
                manualText:      qsTr("Press to move control surfaces. Did they work properly?")
                onPressed: if (_manualState != _statePassed){_activeVehicle.preflightServoTest();}
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
                name:        qsTr("Payload")
                manualText:  qsTr("Press to capture image. Image captured?")

                onPressed:   if (_manualState != _statePassed){_activeVehicle.triggerSimpleCamera()}
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

