/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs  1.2
import QtQuick.Layouts  1.2

import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Vehicle       1.0
import QGroundControl.Controls      1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl.Palette       1.0

// Editor for Fixed Wing Landing Pattern complex mission item
Rectangle {
    id:         _root
    height:     visible ? ((editorColumn.visible ? editorColumn.height : editorColumnNeedLandingPoint.height) + (_margin * 2)) : 0
    width:      availableWidth
    color:      qgcPal.windowShadeDark
    radius:     _radius

    // The following properties must be available up the hierarchy chain
    //property real   availableWidth    ///< Width for control
    //property var    missionItem       ///< Mission Item for editor

    property real   _margin:                    ScreenTools.defaultFontPixelWidth / 2
    property real   _spacer:                    ScreenTools.defaultFontPixelWidth / 2
    property var    _activeVehicle:             QGroundControl.multiVehicleManager.activeVehicle
    property string _setToVehicleHeadingStr:    qsTr("Set to vehicle heading")
    property string _setToVehicleLocationStr:   qsTr("Set to vehicle location")


    ExclusiveGroup { id: distanceGlideGroup }

    Column {
        id:                 editorColumn
        anchors.margins:    _margin
        anchors.left:       parent.left
        anchors.right:      parent.right
        spacing:            _margin
        visible:            missionItem.landingCoordSet

        SectionHeader {
            id:     loiterPointSection
            text:   qsTr("Loiter point")
        }

        Column {
            anchors.left:       parent.left
            anchors.right:      parent.right
            spacing:            _margin
            visible:            loiterPointSection.checked

            Item { width: 1; height: _spacer }

            RowLayout{

                anchors.right:  parent.right
                anchors.left:   parent.left

               QGCLabel { text: qsTr("Altitude") }

                FactTextField {
                    Layout.fillWidth:   true
                    fact:               missionItem.loiterAltitude
                }
            QGCButton {
                Layout.preferredWidth:  height/1.5
                text:                   "-"
                onClicked: {
                missionItem.loiterAltitude.rawValue = missionItem.loiterAltitude.rawValue - 5;
            }
            }

            QGCButton {
                    Layout.preferredWidth:  height/1.5
                    text:                   "+"
                    onClicked: {
                    missionItem.loiterAltitude.rawValue = missionItem.loiterAltitude.rawValue + 5;
                }
            }
            }
            RowLayout{

                anchors.right:  parent.right
                anchors.left:   parent.left

               QGCLabel { text: qsTr("Radius") }

                FactTextField {
                    Layout.fillWidth:   true
                    fact:               missionItem.loiterRadius
                }
            QGCButton {
                Layout.preferredWidth:  height/1.5
                text:                   "-"
                onClicked: {
                missionItem.loiterRadius.rawValue = missionItem.loiterRadius.rawValue - 5;
            }
            }

            QGCButton {
                    Layout.preferredWidth:  height/1.5
                    text:                   "+"
                    onClicked: {
                    missionItem.loiterRadius.rawValue = missionItem.loiterRadius.rawValue + 5;
                }
            }
            }


            Item { width: 1; height: _spacer }

            QGCCheckBox {
                text:           qsTr("Loiter clockwise")
                checked:        missionItem.loiterClockwise
                onClicked:      missionItem.loiterClockwise = checked
            }

            QGCButton {
                text:       _setToVehicleHeadingStr
                visible:    _activeVehicle
                onClicked:  missionItem.landingHeading.rawValue = _activeVehicle.heading.rawValue
            }
        }

        SectionHeader {
            id:     transitionPointSection
            text:   qsTr("Transition point")
        }
        Column {


            anchors.left:       parent.left
            anchors.right:      parent.right
            spacing:            _margin
            visible:            transitionPointSection.checked

            Item { width: 1; height: _spacer }

            RowLayout{

                anchors.right:  parent.right
                anchors.left:   parent.left

               QGCLabel { text: qsTr("Altitude") }

                FactTextField {
                    Layout.fillWidth:   true
                    fact:               missionItem.transitionAltitude
                }
            QGCButton {
                Layout.preferredWidth:  height/1.5
                text:                   "-"
                onClicked: {
                missionItem.transitionAltitude.rawValue = missionItem.transitionAltitude.rawValue - 5;
            }
            }

            QGCButton {
                    Layout.preferredWidth:  height/1.5
                    text:                   "+"
                    onClicked: {
                    missionItem.transitionAltitude.rawValue = missionItem.transitionAltitude.rawValue + 5;
                }
            }
            }
        }
        SectionHeader {
            id:     landingPointSection
            text:   qsTr("Landing point")
        }


        Column {
            anchors.left:       parent.left
            anchors.right:      parent.right
            spacing:            _margin
            visible:            landingPointSection.checked

            Item { width: 1; height: _spacer }

            GridLayout {
                anchors.left:    parent.left
                anchors.right:   parent.right
                columns:         2

                QGCLabel { text: qsTr("Heading") }

                FactTextField {
                    Layout.fillWidth:   true
                    fact:               missionItem.landingHeading
                }

                QGCLabel { text: qsTr("Altitude") }

                FactTextField {
                    Layout.fillWidth:   true
                    fact:               missionItem.landingAltitude
                }

            }
        }
        SectionHeader {
            id:      landingDistance
            text:   qsTr("Landing Distance")
            checked: true
        }


        RowLayout{
            visible:            landingPointSection.checked

            anchors.right:  parent.right
            anchors.left:   parent.left

            QGCRadioButton {
                id:                 specifyLandingDistance
                checked:            missionItem.valueSetIsDistance.rawValue
                exclusiveGroup:     distanceGlideGroup
                onClicked:          missionItem.valueSetIsDistance.rawValue = checked
                Layout.fillWidth:   true
            }

            FactTextField {
                fact:               missionItem.landingDistance
                enabled:            specifyLandingDistance.checked
                Layout.fillWidth:   true
            }

        QGCButton {
            Layout.preferredWidth:  height/1.5
            text:                   "-"
            onClicked: {
            missionItem.landingDistance.rawValue = missionItem.landingDistance.rawValue - 10;
        }
        }

        QGCButton {
                Layout.preferredWidth:  height/1.5
                text:                   "+"
                onClicked: {
                missionItem.landingDistance.rawValue = missionItem.landingDistance.rawValue + 10;
            }
        }
        }



             SectionHeader {
                 id:     glideSlopeSection
                 text:   qsTr("Glide Slope")
                 checked: true
             }
        RowLayout{
            visible:            landingPointSection.checked

            anchors.right:  parent.right
            anchors.left:   parent.left

            QGCRadioButton {
                id:                 specifyGlideSlope
                checked:            !missionItem.valueSetIsDistance.rawValue
                exclusiveGroup:     distanceGlideGroup
                onClicked:          missionItem.valueSetIsDistance.rawValue = !checked
                Layout.fillWidth:   true
            }

           FactTextField {
               fact:               missionItem.glideSlope
               enabled:            specifyGlideSlope.checked
               Layout.fillWidth:   true
           }

        QGCButton {
            Layout.preferredWidth:  height/1.5
            text:                   "-"
            onClicked: {
            missionItem.glideSlope.rawValue = missionItem.glideSlope.rawValue - 1;
        }
        }

        QGCButton {
                Layout.preferredWidth:  height/1.5
                text:                   "+"
                onClicked: {
                missionItem.glideSlope.rawValue = missionItem.glideSlope.rawValue + 1;
            }
        }
        }


        QGCButton {
            text:               _setToVehicleLocationStr
            visible:            _activeVehicle
            Layout.columnSpan:  2
            onClicked:          missionItem.landingCoordinate = _activeVehicle.coordinate
        }

        Item { width: 1; height: _spacer }

        QGCCheckBox {
            anchors.right:  parent.right
            text:           qsTr("Altitudes relative to home")
            checked:        missionItem.altitudesAreRelative
            visible:        QGroundControl.corePlugin.options.showMissionAbsoluteAltitude || !missionItem.altitudesAreRelative
            onClicked:      missionItem.altitudesAreRelative = checked
        }
    }

    Column {
        id:                 editorColumnNeedLandingPoint
        anchors.margins:    _margin
        anchors.top:        parent.top
        anchors.left:       parent.left
        anchors.right:      parent.right
        visible:            !missionItem.landingCoordSet
        spacing:            ScreenTools.defaultFontPixelHeight

        QGCLabel {
            anchors.left:           parent.left
            anchors.right:          parent.right
            wrapMode:               Text.WordWrap
            horizontalAlignment:    Text.AlignHCenter
            text:                   qsTr("Click in map to set landing point.")
        }

        QGCLabel {
            anchors.left:           parent.left
            anchors.right:          parent.right
            horizontalAlignment:    Text.AlignHCenter
            text:                   qsTr("- or -")
            visible:                _activeVehicle
        }
        QGCButton {
                   anchors.horizontalCenter:   parent.horizontalCenter
                   text:                       _setToVehicleLocationStr
                   visible:                    _activeVehicle

                   onClicked: {
                       missionItem.landingCoordinate = _activeVehicle.coordinate
                       missionItem.landingHeading.rawValue = _activeVehicle.heading.rawValue
                   }
               }

    }
}
