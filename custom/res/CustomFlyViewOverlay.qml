/****************************************************************************
 *
 * (c) 2009-2019 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 * @file
 *   @author Gus Grubba <gus@auterion.com>
 */

import QtQuick          2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts  1.11

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.Palette       1.0
import QGroundControl.ScreenTools   1.0

import Custom.Widgets 1.0

Item {
    property var parentToolInsets                       // These insets tell you what screen real estate is available for positioning the controls in your overlay
    property var totalToolInsets:   _totalToolInsets    // The insets updated for the custom overlay additions
    property var mapControl

    readonly property string noGPS:         qsTr("NO GPS")
    readonly property real   indicatorValueWidth:   ScreenTools.defaultFontPixelWidth * 2

    property var    _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property real   _indicatorDiameter:     ScreenTools.defaultFontPixelWidth * 18
    property real   _indicatorsHeight:      ScreenTools.defaultFontPixelHeight
    property var    _sepColor:              qgcPal.globalTheme === QGCPalette.Light ? Qt.rgba(0,0,0,0.5) : Qt.rgba(1,1,1,0.5)
    property color  _indicatorsColor:       qgcPal.text
    property bool   _isVehicleGps:          _activeVehicle ? _activeVehicle.gps.count.rawValue > 1 && _activeVehicle.gps.hdop.rawValue < 1.4 : false
    property string _altitude:              _activeVehicle ? (isNaN(_activeVehicle.altitudeRelative.value) ? "0.0" : _activeVehicle.altitudeRelative.value.toFixed(1)) + ' ' + _activeVehicle.altitudeRelative.units : "0.0"
    property string _distanceStr:           isNaN(_distance) ? "0" : _distance.toFixed(0) + ' ' + QGroundControl.unitsConversion.appSettingsHorizontalDistanceUnitsString
    property real   _heading:               _activeVehicle   ? _activeVehicle.heading.rawValue : 0
    property real   _distance:              _activeVehicle ? _activeVehicle.distanceToHome.rawValue : 0
    property string _messageTitle:          ""
    property string _messageText:           ""
    property real   _toolsMargin:           ScreenTools.defaultFontPixelWidth * 0.75

    function secondsToHHMMSS(timeS) {
        var sec_num = parseInt(timeS, 10);
        var hours   = Math.floor(sec_num / 3600);
        var minutes = Math.floor((sec_num - (hours * 3600)) / 60);
        var seconds = sec_num - (hours * 3600) - (minutes * 60);
        if (hours   < 10) {hours   = "0"+hours;}
        if (minutes < 10) {minutes = "0"+minutes;}
        if (seconds < 10) {seconds = "0"+seconds;}
        return hours+':'+minutes+':'+seconds;
    }

    QGCToolInsets {
        id:                     _totalToolInsets
    }

    //-------------------------------------------------------------------------
    //-- Vehicle Indicator
    Rectangle {

        id:                     vehicleIndicator
        color:                  qgcPal.window
        opacity:                .8
        width:                  -anchors.rightMargin + vehicleStatusGrid.width  + (ScreenTools.defaultFontPixelWidth) + (compassBezel.width * .75)
        height:                 vehicleStatusGrid.height + (ScreenTools.defaultFontPixelHeight * 1.5)
        radius:                 30
        //anchors.bottomMargin:   parentToolInsets.bottomEdgeRightInset
        anchors.top:         parent.top
        anchors.topMargin:   _toolsMargin
        anchors.right:          attitudeIndicator.left
        anchors.rightMargin:    -attitudeIndicator.width / 2

        GridLayout {
            id:                     vehicleStatusGrid
            columnSpacing:          ScreenTools.defaultFontPixelWidth  * 1.5
            rowSpacing:             ScreenTools.defaultFontPixelHeight * 0.5
            columns:                5
            anchors.centerIn:       parent

            //-- Compass
            Item {
                Layout.rowSpan:         3
                Layout.column:          4
                Layout.minimumWidth:    parent.height * 1.25
                Layout.fillHeight:      true
                Layout.fillWidth:       true
                //-- Large circle
                Rectangle {
                    id:                 compassBezel
                    height:             parent.height
                    width:              height
                    radius:             height * 0.5
                    border.color:       qgcPal.text
                    border.width:       1
                    color:              Qt.rgba(0,0,0,0)
                    anchors.centerIn:   parent
                }
                //-- North Label
                Rectangle {
                    height:             ScreenTools.defaultFontPixelHeight * 0.75
                    width:              ScreenTools.defaultFontPixelWidth  * 2
                    radius:             ScreenTools.defaultFontPixelWidth  * 0.25
                    color:              qgcPal.windowShade
                    anchors.top:        parent.top
                    anchors.topMargin:  ScreenTools.defaultFontPixelHeight * -0.25
                    anchors.horizontalCenter: parent.horizontalCenter
                    QGCLabel {
                        text:               "N"
                        color:              qgcPal.colorRed
                        font.pointSize:     ScreenTools.smallFontPointSize
                        anchors.centerIn:   parent
                    }
                }
                //-- Needle
                Image {
                    id:                 compassNeedle
                    anchors.centerIn:   parent
                    height:             parent.height * 0.75
                    width:              height
                    source:             "/custom/img/compass_needle.svg"
                    fillMode:           Image.PreserveAspectFit
                    sourceSize.height:  height
                    transform: [
                        Rotation {
                            origin.x:   compassNeedle.width  / 2
                            origin.y:   compassNeedle.height / 2
                            angle:      _heading
                        }]
                }
                //-- Heading
                Rectangle {
                    height:             ScreenTools.defaultFontPixelHeight * 0.75
                    width:              ScreenTools.defaultFontPixelWidth  * 3.5
                    radius:             ScreenTools.defaultFontPixelWidth  * 0.25
                    color:              qgcPal.windowShade
                    anchors.bottom:         parent.bottom
                    anchors.bottomMargin:   ScreenTools.defaultFontPixelHeight * -0.25
                    anchors.horizontalCenter: parent.horizontalCenter
                    QGCLabel {
                        text:               _heading
                        color:              qgcPal.colorRed
                        font.pointSize:     ScreenTools.smallFontPointSize
                        anchors.centerIn:   parent
                    }
                }
            }
            //-- Second Row
            //-- Chronometer
            QGCColoredImage {
                height:                 _indicatorsHeight
                width:                  height
                source:                 "/custom/img/chronometer.svg"
                fillMode:               Image.PreserveAspectFit
                sourceSize.height:      height
                Layout.alignment:       Qt.AlignVCenter | Qt.AlignHCenter
                color:                  qgcPal.text
            }
            QGCLabel {
                id:                     firstLabel
                text: {
                    if(_activeVehicle)
                        return secondsToHHMMSS(_activeVehicle.getFact("flightTime").value)
                    return "00:00:00"
                }
                color:                  _indicatorsColor
                font.pointSize:         ScreenTools.smallFontPointSize
                Layout.fillWidth:       true
                Layout.minimumWidth:    indicatorValueWidth
                horizontalAlignment:    Text.AlignLeft
            }
            //-- Air Speed
            QGCColoredImage {
                height:                 _indicatorsHeight
                width:                  height
                source:                 "/custom/img/horizontal_speed.svg"
                fillMode:               Image.PreserveAspectFit
                sourceSize.height:      height
                Layout.alignment:       Qt.AlignVCenter | Qt.AlignHCenter
                color:                  qgcPal.text
            }
            QGCLabel {
                text:                   _activeVehicle ? _activeVehicle.airSpeed.value.toFixed(1) + ' ' + _activeVehicle.groundSpeed.units : "0.0"
                color:                  _indicatorsColor
                font.pointSize:         ScreenTools.smallFontPointSize
                Layout.fillWidth:       true
                Layout.minimumWidth:    indicatorValueWidth
                horizontalAlignment:    firstLabel.horizontalAlignment
            }
            //-- Altitude
            QGCColoredImage {
                height:                 _indicatorsHeight
                width:                  height
                source:                 "/custom/img/altitude.svg"
                fillMode:               Image.PreserveAspectFit
                sourceSize.height:      height
                Layout.alignment:       Qt.AlignVCenter | Qt.AlignHCenter
                color:                  qgcPal.text

            }
            QGCLabel {
                text:                   _altitude
                color:                  _indicatorsColor
                font.pointSize:         ScreenTools.smallFontPointSize
                Layout.fillWidth:       true
                Layout.minimumWidth:    indicatorValueWidth
                horizontalAlignment:    firstLabel.horizontalAlignment
            }
            //-- Distance
            QGCColoredImage {
                height:                 _indicatorsHeight
                width:                  height
                source:                 "/custom/img/distance.svg"
                fillMode:               Image.PreserveAspectFit
                sourceSize.height:      height
                Layout.alignment:       Qt.AlignVCenter | Qt.AlignHCenter
                color:                  qgcPal.text

            }
            QGCLabel {
                text:                   _distance ? _distanceStr : noGPS
                color:                  _distance ? _indicatorsColor : qgcPal.colorOrange
                font.pointSize:         ScreenTools.smallFontPointSize
                Layout.fillWidth:       true
                Layout.minimumWidth:    indicatorValueWidth
                horizontalAlignment:    firstLabel.horizontalAlignment
            }
        }
    }
    //-------------------------------------------------------------------------
    //-- Attitude Indicator
    Rectangle {
        color:                  "#21252900"
        width:                  attitudeIndicator.width * 0.5
        height:                 vehicleIndicator.height
        anchors.top:            vehicleIndicator.top
        anchors.left:           vehicleIndicator.right
    }
    Rectangle {
        id:                     attitudeIndicator
        anchors.top:         vehicleIndicator.top
        anchors.topMargin:   ScreenTools.defaultFontPixelWidth * -0.5
        anchors.right:          parent.right
        anchors.rightMargin:    _toolsMargin
        height:                 ScreenTools.defaultFontPixelHeight * 6
        width:                  height
        radius:                 height * 0.5
        color:                  "#21252900"
        CustomAttitudeWidget {
            size:               parent.height * 0.95
            vehicle:            _activeVehicle
            showHeading:        false
            anchors.centerIn:   parent
        }
    }
}
