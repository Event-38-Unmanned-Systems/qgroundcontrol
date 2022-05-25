/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick                      2.12
import QtQuick.Layouts              1.12

import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Vehicle       1.0
import QGroundControl.Controls      1.0
import QGroundControl.Palette       1.0

Rectangle {
    id:                 telemetryPanel
    height:             telemetryLayout.height + (_toolsMargin * 2)
    width:              telemetryLayout.width + (_toolsMargin * 2)
    color:              "#00000000"
    radius:             ScreenTools.defaultFontPixelWidth / 2

    property bool       bottomMode: true

    DeadMouseArea { anchors.fill: parent }

    ColumnLayout {
        id:                 telemetryLayout
        anchors.margins:    _toolsMargin
        anchors.bottom:     parent.bottom
        anchors.left:       parent.left


        GuidedActionConfirm {
            Layout.fillWidth:   true
            guidedController:   _guidedController
            altitudeSlider:     _guidedAltSlider
        }
    }
}
