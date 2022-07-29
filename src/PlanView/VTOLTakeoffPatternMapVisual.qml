/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.3
import QtQuick.Controls 1.2
import QtLocation       5.3
import QtPositioning    5.3
import QtQuick.Layouts  1.11
import QtQuick.Shapes   1.12
import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0
import QGroundControl.FlightMap     1.0

Item {
    id: _root
    property var map        ///< Map control to place item in

    signal clicked(int sequenceNumber)

    readonly property real _landingWidthMeters:     5
    readonly property real _landingLengthMeters:    40

    property var    _missionItem:               object
    property var    _mouseArea
    property var    _dragAreas:                 [ ]
    property var    _flightPath
    property var    _transitionBeginPath
    property var    _transitionFinishPath
    property var    _vtolTakeoffPointObject
    property var    _climboutPointObject
    property bool   _useLoiterToAlt:            _missionItem.useLoiterToAlt.rawValue
    property real   _takeoffBearing:            _missionItem.vtolTakeoffCoordinate.azimuthTo(_missionItem.climboutCoordinate)
    property real   _midSlopeAltitudeMeters

    function hideItemVisuals() {
        objMgr.destroyObjects()
    }

    function _calcGlideSlopeHeights() {

        var adjacent = _missionItem.vtolTakeoffCoordinate.distanceTo(_missionItem.climboutCoordinate) - 120;

        var opposite = _missionItem.climboutAlt.rawValue - _missionItem.vtolAlt.rawValue
        var angleRadians = Math.atan(opposite / adjacent)
        var glideSlopeDistance = adjacent

        _midSlopeAltitudeMeters = Math.tan(angleRadians) * (glideSlopeDistance / 2) + _missionItem.vtolAlt.rawValue

    }

    function showItemVisuals() {
        if (objMgr.rgDynamicObjects.length === 0) {
            _vtolTakeoffPointObject = objMgr.createObject(vtolTakeoffPointComponent, map, true /* parentObjectIsMap */)
            _climboutPointObject = objMgr.createObject(climboutPointComponent, map, true /* parentObjectIsMap */)
            var rgComponents = [ flightPathComponent, transitionBeginPathComponent,transitionFinishPathComponent, loiterRadiusComponent , rotationIndicatorComponentTop,rotationIndicatorComponentBottom, midGlideSlopeHeightComponent]
            objMgr.createObjects(rgComponents, map, true /* parentObjectIsMap */)
        }
    }

    function hideMouseArea() {
        if (_mouseArea) {
            _mouseArea.destroy()
            _mouseArea = undefined
        }
    }

    function showMouseArea() {
        if (!_mouseArea) {
            _mouseArea = mouseAreaComponent.createObject(map)
            map.addMapItem(_mouseArea)
        }
    }

    function hideDragAreas() {
        for (var i=0; i<_dragAreas.length; i++) {
            _dragAreas[i].destroy()
        }
        _dragAreas = [ ]
    }

    function showDragAreas() {
        if (_dragAreas.length === 0) {
            _dragAreas.push(vtoltakeoffDragAreaComponent.createObject(map))
            _dragAreas.push(climboutDragAreaComponent.createObject(map))
        }
    }

    function _setFlightPath() {
            _flightPath = [ _missionItem.vtolTakeoffCoordinate, _missionItem.climboutCoordinate ]
            _setTransitionPath()
    }

    function _setTransitionPath() {

            var coord1 = _missionItem.vtolTakeoffCoordinate.atDistanceAndAzimuth(80,_takeoffBearing)
            var coord2 = _missionItem.vtolTakeoffCoordinate.atDistanceAndAzimuth(120,_takeoffBearing)

            _transitionBeginPath = [ _missionItem.vtolTakeoffCoordinate,coord1]
            _transitionFinishPath = [coord1,coord2]

    }

    QGCDynamicObjectManager {
        id: objMgr
    }

    Component.onCompleted: {
        if (_missionItem.takeoffCoordSet) {
            showItemVisuals()
            if (!_missionItem.flyView && _missionItem.isCurrentItem) {
                showDragAreas()
            }
            _setFlightPath()
        } else if (!_missionItem.flyView && _missionItem.isCurrentItem) {
            showMouseArea()
        }
    }

    Component.onDestruction: {
        hideDragAreas()
        hideMouseArea()
        hideItemVisuals()
    }

    on_UseLoiterToAltChanged:{
        _calcGlideSlopeHeights()
        _setFlightPath()
    }

    Connections {
        target: _missionItem

        onIsCurrentItemChanged: {
            if (_missionItem.flyView) {
                return
            }
            if (_missionItem.isCurrentItem) {
                if (_missionItem.takeoffCoordSet) {
                    showDragAreas()
                } else {
                    showMouseArea()
                }
            } else {
                hideMouseArea()
                hideDragAreas()
            }
        }

        onTakeoffCoordSetChanged: {
            _missionItem.wizardMode = false;
            if (_missionItem.flyView) {
                return
            }
            if (_missionItem.takeoffCoordSet) {
                hideMouseArea()
                showItemVisuals()
                showDragAreas()
                _setFlightPath()
            } else if (_missionItem.isCurrentItem) {
                hideDragAreas()
                showMouseArea()
            }
        }

        onClimboutCoordinateChanged:{
            _calcGlideSlopeHeights()
            _setFlightPath()
        }

        onVtolTakeoffCoordinateChanged:{
            _calcGlideSlopeHeights()
            _setFlightPath()
        }
    }


    Component {
        id: midGlideSlopeHeightComponent

        MapQuickItem {
            anchorPoint.x:  sourceItem.width / 2
            anchorPoint.y:  0
            z:              QGroundControl.zOrderMapItems
            visible:        _missionItem.isCurrentItem

            sourceItem: HeightIndicator {
                map:        _root.map
                heightText: Math.floor(QGroundControl.unitsConversion.metersToAppSettingsHorizontalDistanceUnits(_midSlopeAltitudeMeters)) +
                            QGroundControl.unitsConversion.appSettingsHorizontalDistanceUnitsString + "<sup>*</sup>"
            }

            function recalc() {
                var halfDistance = (_missionItem.vtolTakeoffCoordinate.distanceTo(_missionItem.climboutCoordinate)-120) / 2
                var centeredCoordinate = _missionItem.climboutCoordinate.atDistanceAndAzimuth(halfDistance,_takeoffBearing+180)
                var angleIncrement = _takeoffBearing > 180 ? -90 : 90
                coordinate = centeredCoordinate.atDistanceAndAzimuth(_landingWidthMeters / 2, _takeoffBearing + angleIncrement)
            }

            Component.onCompleted: recalc()

            Connections {
                target:                             _missionItem
                onClimboutCoordinateChanged:         recalc()
                onVtolTakeoffCoordinateChanged:      recalc()

            }

            Connections {
                target:             _missionItem.useLoiterToAlt
                onRawValueChanged:  recalc()
            }
        }
    }

    // Mouse area to capture takeoff point coordindate
    Component {
        id:  mouseAreaComponent

        MouseArea {
            anchors.fill:   map
            z:              QGroundControl.zOrderMapItems + 1   // Over item indicators

            readonly property int   _decimalPlaces:             8

            onClicked: {
                var coordinate = map.toCoordinate(Qt.point(mouse.x, mouse.y), false /* clipToViewPort */)
                coordinate.latitude = coordinate.latitude.toFixed(_decimalPlaces)
                coordinate.longitude = coordinate.longitude.toFixed(_decimalPlaces)
                coordinate.altitude = coordinate.altitude.toFixed(_decimalPlaces)
                _missionItem.climboutCoordinate = coordinate
            }
        }
    }

    // Control which is used to drag the final approach point
    Component {
        id: vtoltakeoffDragAreaComponent

        MissionItemIndicatorDrag {
            mapControl:     _root.map
            itemIndicator:  _vtolTakeoffPointObject
            itemCoordinate: _missionItem.vtolTakeoffCoordinate

            property bool _preventReentrancy: false

            onItemCoordinateChanged: {


                if (!globals.activeVehicle){
                  _missionItem.vtolTakeoffCoordinate = itemCoordinate
                }
                /*if (!_preventReentrancy) {
                    _preventReentrancy = true;
                _preventReentrancy = false;
                  } */
            }
        }
    }

    // Control which is used to drag the landing point
    Component {
        id: climboutDragAreaComponent

        MissionItemIndicatorDrag {
            mapControl:     _root.map
            itemIndicator:  _climboutPointObject
            itemCoordinate: _missionItem.climboutCoordinate
            property bool _preventReentrancy: false

        onItemCoordinateChanged: {
            var angle = _missionItem.vtolTakeoffCoordinate.azimuthTo(itemCoordinate)
            var distance = _missionItem.vtolTakeoffCoordinate.distanceTo(itemCoordinate)
            var minDist =  _useLoiterToAlt ? _missionItem.takeoffDist.rawMin + _missionItem.loiterRadius.rawValue :  _missionItem.takeoffDist.rawMin

            if (!_preventReentrancy) {
                    if (distance < minDist ){
                        if (!_preventReentrancy) {
                            if (Drag.active) {
                                _preventReentrancy = true
                                _missionItem.climboutCoordinate = _missionItem.vtolTakeoffCoordinate.atDistanceAndAzimuth(minDist, angle)
                                _preventReentrancy = false
                            }
                        }
                    }

                    else if (distance > _missionItem.takeoffDist.rawMax){
                        if (!_preventReentrancy) {
                            if (Drag.active) {
                                _preventReentrancy = true
                                _missionItem.climboutCoordinate = _missionItem.vtolTakeoffCoordinate.atDistanceAndAzimuth(_missionItem.takeoffDist.rawMax, angle)
                                _preventReentrancy = false
                            }
                        }
                    }
                    else { if (!_preventReentrancy) {
                            if (Drag.active) {
                                _preventReentrancy = true
                                _missionItem.climboutCoordinate = itemCoordinate
                                _preventReentrancy = false
                            }
                        }
                       }
            }
        }
    }
}

    // Flight path
    Component {
        id: flightPathComponent

        MapPolyline {
            z:          QGroundControl.zOrderMapItems - 1   // Under item indicators
            line.color: (_missionItem.isCurrentItem) ? "#20cc02" : "#be781c"
            line.width: 2
            path:       _flightPath
        }
    }

    // Flight path
    Component {
        id: transitionBeginPathComponent

        MapPolyline {
            z:          QGroundControl.zOrderMapItems - 1  // Under item indicators
            line.color: (_missionItem.isCurrentItem) ? "#fff133" : "#be781c"
            line.width: 2
            path:       _transitionBeginPath
        }
    }
    // Flight path
    Component {
        id: transitionFinishPathComponent

        MapPolyline {
            z:          QGroundControl.zOrderMapItems - 1   // Under item indicators
            line.color: (_missionItem.isCurrentItem) ? "#a7ff33" : "#be781c"
            line.width: 2
            path:       _transitionFinishPath
        }
    }

    // Vtol takeoff point
    Component {
        id: vtolTakeoffPointComponent

        MapQuickItem {
            anchorPoint.x:  sourceItem.anchorPointX
            anchorPoint.y:  sourceItem.anchorPointY
            coordinate:     _missionItem.vtolTakeoffCoordinate
            z:              QGroundControl.zOrderMapItems

            sourceItem:
                MissionItemIndexLabel {
                index:      _missionItem.sequenceNumber
                label:      ("VTOL Takeoff")
                checked:    _missionItem.isCurrentItem

                onClicked: _root.clicked(_missionItem.sequenceNumber)
            }
        }
    }

    // climbout point
    Component {
        id: climboutPointComponent

        MapQuickItem {
            anchorPoint.x:  sourceItem.anchorPointX
            anchorPoint.y:  sourceItem.anchorPointY
            coordinate:     _missionItem.climboutCoordinate
            z:              QGroundControl.zOrderMapItems

            sourceItem:
                MissionItemIndexLabel {
                index:      _missionItem.sequenceNumber + 1
                label:      qsTr("Climb Out")
                checked:    _missionItem.isCurrentItem

                onClicked: _root.clicked(_missionItem.sequenceNumber)
            }
        }
    }

    Component {
        id: loiterRadiusComponent

        MapCircle {
            z:              QGroundControl.zOrderMapItems
            center:         _missionItem.climboutCoordinate
            radius:         _missionItem.loiterRadius.rawValue
            border.width:   2
            border.color:   "green"
            color:          "transparent"
            visible:        _useLoiterToAlt
        }
    }

    Component {
        id: rotationIndicatorComponentTop

        MapQuickItem {
            visible: _useLoiterToAlt
            property bool topIndicator: true

            function updateCoordinate() {
                coordinate = _missionItem.climboutCoordinate.atDistanceAndAzimuth(_missionItem.loiterRadius.rawValue, topIndicator ? 0 : 180)
            }

            Connections {
                target:                             _missionItem
                onClimboutCoordinateChanged:         updateCoordinate()
                onVtolTakeoffCoordinateChanged:      updateCoordinate()
            }

           Component.onCompleted: updateCoordinate()

            sourceItem: Shape {
                width:            ScreenTools.defaultFontPixelHeight/ 1.5
                height:           ScreenTools.defaultFontPixelHeight/ 1.5
                anchors.centerIn: parent

                transform: Rotation {
                    origin.x: width / 2
                    origin.y: height / 2
                    angle:   (_missionItem.loiterClockwise.rawValue ? 0 : 180) + (topIndicator ? 180 : 0)
                }

                ShapePath {
                    strokeWidth: 2
                    strokeColor: "green"
                    fillColor:   "green"
                    startX:      0
                    startY:      width / 2
                    PathLine { x: width;  y: width     }
                    PathLine { x: width;  y: 0         }
                    PathLine { x: 0;      y: width / 2 }
                }

            }
        }
    }
    Component {
        id: rotationIndicatorComponentBottom

        MapQuickItem {
            visible: _useLoiterToAlt
            property bool topIndicator: false

            function updateCoordinate() {
                coordinate = _missionItem.climboutCoordinate.atDistanceAndAzimuth(_missionItem.loiterRadius.rawValue, topIndicator ? 0 : 180)
            }

            Connections {
                target:                             _missionItem
                onClimboutCoordinateChanged:         updateCoordinate()
                onVtolTakeoffCoordinateChanged:      updateCoordinate()
            }

           Component.onCompleted: updateCoordinate()

            sourceItem: Shape {
                width:            ScreenTools.defaultFontPixelHeight / 1.5
                height:           ScreenTools.defaultFontPixelHeight / 1.5
                anchors.centerIn: parent

                transform: Rotation {
                    origin.x: width / 2
                    origin.y: height / 2
                    angle:   (_missionItem.loiterClockwise.rawValue ? 0 : 180) + (topIndicator ? 180 : 0)
                }

                ShapePath {
                    strokeWidth: 2
                    strokeColor: "green"
                    fillColor:   "green"
                    startX:      0
                    startY:      width / 2
                    PathLine { x: width;  y: width     }
                    PathLine { x: width;  y: 0         }
                    PathLine { x: 0;      y: width / 2 }
                }

            }
        }
    }


}
