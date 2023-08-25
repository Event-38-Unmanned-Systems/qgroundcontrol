/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.11
import QtQuick.Layouts  1.11

import QGroundControl                       1.0
import QGroundControl.Controls              1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.Palette               1.0
import MAVLink                              1.0

//-------------------------------------------------------------------------
//-- Battery Indicator
Item {
    id:             _root
    anchors.top:    parent.top
    anchors.bottom: parent.bottom
    width:          batteryIndicatorRow.width
    property var mahRemaining: -5
    property bool showIndicator: true
    property int cell: 0
    property int maxMah: 0
    property bool reCalc: false

    property var _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle


    Row {
        id:             batteryIndicatorRow
        anchors.top:    parent.top
        anchors.bottom: parent.bottom

        Repeater {
            model: _activeVehicle ? _activeVehicle.batteries : 0

            Loader {
                anchors.top:        parent.top
                anchors.bottom:     parent.bottom
                sourceComponent:    batteryVisual

                property var battery: object
            }
        }
    }
    MouseArea {
        anchors.fill:   parent
        onClicked: {
            mainWindow.showIndicatorPopup(_root, batteryPopup)
        }
    }

    Component {
        id: batteryVisual

        Row {
            anchors.top:    parent.top
            anchors.bottom: parent.bottom

            function getBatteryColor() {
                switch (battery.chargeState.rawValue) {
                case MAVLink.MAV_BATTERY_CHARGE_STATE_OK:
                    return qgcPal.text
                case MAVLink.MAV_BATTERY_CHARGE_STATE_LOW:
                    return qgcPal.colorOrange
                case MAVLink.MAV_BATTERY_CHARGE_STATE_CRITICAL:
                case MAVLink.MAV_BATTERY_CHARGE_STATE_EMERGENCY:
                case MAVLink.MAV_BATTERY_CHARGE_STATE_FAILED:
                case MAVLink.MAV_BATTERY_CHARGE_STATE_UNHEALTHY:
                    return qgcPal.colorRed
                default:
                    return qgcPal.text
                }
            }
            /*these are estimated averages determined from a 100
            minute endurance flight on the E400 used to determine the starting
            estimated capicity of the flight battery*/

            function getMahRemaining(cell1,maxMah1) {

                var mahUsed

                //array containing the volatage used for voltage to MAH conversion for SSLion batteries.
                var battStageVolt = [4.075 * cell1,3.7325 * cell1,3.5275 * cell1, 3.37 * cell1, 3 * cell1,0]
                var battStageScaler =[.3445,.2375,.1783,.2395]
                var battStageMah = [(maxMah1 * battStageScaler[0]),(maxMah1 * battStageScaler[1]),(maxMah1 * battStageScaler[2]),(maxMah1 * battStageScaler[3])]
                //full
            if (battery.voltage.rawValue > battStageVolt[0]){
                //~100% battery over this voltage
                mahUsed = 0
            }
            //stage1
            else if (battery.voltage.rawValue >= battStageVolt[1]){
            //~5512 mah in this range max
            mahUsed = battStageMah[0] * ((battStageVolt[0] - battery.voltage.rawValue) / (battStageVolt[0] - battStageVolt[1]))
            }
            //stage2
            else if (battery.voltage.rawValue >= battStageVolt[2]){
                //~3800 mah
                mahUsed = battStageMah[0] + (battStageMah[1]  * (battStageVolt[1] - battery.voltage.rawValue) / (battStageVolt[1] - battStageVolt[2]))
            }
            //stage3
            else if (battery.voltage.rawValue >= battStageVolt[3]){
              //~2854 mah left
                mahUsed = battStageMah[0] + battStageMah[1] + (battStageMah[2]  * (battStageVolt[2] - battery.voltage.rawValue) / (battStageVolt[2] - battStageVolt[3]))
            }
            //stage4
            else if (battery.voltage.rawValue >= battStageVolt[4]){
              //~3832 mah left
                mahUsed = battStageMah[0] + battStageMah[1] + battStageMah[2] + (battStageMah[3]  * (battStageVolt[3] - battery.voltage.rawValue) / (battStageVolt[3] - battStageVolt[4]))
            }
            //stage5
            else if (battery.voltage.rawValue >= 0){
              //0 mah left
                mahUsed = maxMah1
            }
            mahRemaining = maxMah1 - mahUsed
            }

            function getBatteryPercentageText() {

                if (!isNaN(battery.voltage.rawValue) && (mahRemaining === -5)){
                     cell = _activeVehicle.batteryCells
                     maxMah = _activeVehicle.batteryMAH
                    getMahRemaining(cell,maxMah)
                }
                else if (cell !== _activeVehicle.batteryCells && mahRemaining !== -5){
                    cell = _activeVehicle.batteryCells
                    maxMah = _activeVehicle.batteryMAH
                    getMahRemaining(cell,maxMah)
                }
                else if (maxMah !==_activeVehicle.batteryMAH && mahRemaining !== -5){
                    cell = _activeVehicle.batteryCells
                    maxMah = _activeVehicle.batteryMAH
                   getMahRemaining(cell,maxMah)
                }
                //handle mod 0 case? Some weird error in here that sometimes
                //happens when connecting to plane. Makes HUD set to NAN
                if (!isNaN(battery.mahConsumed.rawValue) && mahRemaining !== -5){
                       var percent = ((mahRemaining - battery.mahConsumed.rawValue)  / _activeVehicle.batteryMAH) * 100
                       if (percent !== 0){
                       percent = percent - (percent % 1)
                       return percent.toString() + battery.percentRemaining.units
                       }
                       else {return ""}
                }
                else {return ""}
            }

            QGCColoredImage {
                anchors.top:        parent.top
                anchors.bottom:     parent.bottom
                width:              height
                sourceSize.width:   width
                source:             "/qmlimages/Battery.svg"
                fillMode:           Image.PreserveAspectFit
                color:              getBatteryColor()
            }

            QGCLabel {
                text:                   getBatteryPercentageText()
                font.pointSize:         ScreenTools.mediumFontPointSize
                color:                  getBatteryColor()
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Component {
        id: batteryValuesAvailableComponent

        QtObject {
            property bool functionAvailable:        battery.function.rawValue !== MAVLink.MAV_BATTERY_FUNCTION_UNKNOWN
            property bool temperatureAvailable:     !isNaN(battery.temperature.rawValue)
            property bool currentAvailable:         !isNaN(battery.current.rawValue)
            property bool mahConsumedAvailable:     !isNaN(battery.mahConsumed.rawValue)
            property bool timeRemainingAvailable:   !isNaN(battery.timeRemaining.rawValue)
            property bool chargeStateAvailable:     battery.chargeState.rawValue !== MAVLink.MAV_BATTERY_CHARGE_STATE_UNDEFINED
        }
    }

    Component {
        id: batteryPopup

        Rectangle {
            width:          mainLayout.width   + mainLayout.anchors.margins * 2
            height:         mainLayout.height  + mainLayout.anchors.margins * 2
            radius:         ScreenTools.defaultFontPixelHeight / 2
            color:          qgcPal.window
            border.color:   qgcPal.text

            ColumnLayout {
                id:                 mainLayout
                anchors.margins:    ScreenTools.defaultFontPixelWidth
                anchors.top:        parent.top
                anchors.right:      parent.right
                spacing:            ScreenTools.defaultFontPixelHeight

                QGCLabel {
                    Layout.alignment:   Qt.AlignCenter
                    text:               qsTr("Battery Status")
                    font.family:        ScreenTools.demiboldFontFamily
                }

                RowLayout {
                    spacing: ScreenTools.defaultFontPixelWidth

                    ColumnLayout {
                        Repeater {
                            model: _activeVehicle ? _activeVehicle.batteries : 0

                            ColumnLayout {
                                spacing: 0

                                property var batteryValuesAvailable: nameAvailableLoader.item

                                Loader {
                                    id:                 nameAvailableLoader
                                    sourceComponent:    batteryValuesAvailableComponent

                                    property var battery: object
                                }

                               // QGCLabel { text: qsTr("Battery %1").arg(object.id.rawValue);    visible: false }
                                //QGCLabel { text: qsTr("Charge State");                          visible: batteryValuesAvailable.chargeStateAvailable }
                               // QGCLabel { text: qsTr("Remaining");                             visible: batteryValuesAvailable.timeRemainingAvailable }
                               // QGCLabel { text: qsTr("Remaining") }
                                QGCLabel { text: qsTr("Voltage") }
                                QGCLabel { text: qsTr("Consumed");                              visible: batteryValuesAvailable.mahConsumedAvailable }
                               // QGCLabel { text: qsTr("Temperature");                           visible: batteryValuesAvailable.temperatureAvailable }
                                //QGCLabel { text: qsTr("Function");                              visible: batteryValuesAvailable.functionAvailable }
                            }
                        }
                    }

                    ColumnLayout {
                        Repeater {
                            model: _activeVehicle ? _activeVehicle.batteries : 0

                            ColumnLayout {
                                spacing: 0

                                property var batteryValuesAvailable: valueAvailableLoader.item

                                Loader {
                                    id:                 valueAvailableLoader
                                    sourceComponent:    batteryValuesAvailableComponent

                                    property var battery: object
                                }

                                //QGCLabel { text: "" }
                                //QGCLabel { text: object.chargeState.enumStringValue;                                        visible: batteryValuesAvailable.chargeStateAvailable }
                                //QGCLabel { text: object.timeRemainingStr.value;                                             visible: batteryValuesAvailable.timeRemainingAvailable }
                                //QGCLabel { text: object.percentRemaining.valueString + " " + object.percentRemaining.units }
                                QGCLabel { text: object.voltage.valueString + " " + object.voltage.units }
                                QGCLabel { text: object.mahConsumed.valueString + " " + object.mahConsumed.units;           visible: batteryValuesAvailable.mahConsumedAvailable }
                                //QGCLabel { text: object.temperature.valueString + " " + object.temperature.units;           visible: batteryValuesAvailable.temperatureAvailable }
                                //QGCLabel { text: object.function.enumStringValue;                                           visible: batteryValuesAvailable.functionAvailable }
                            }
                        }
                    }
                }
            }
        }
    }
}
