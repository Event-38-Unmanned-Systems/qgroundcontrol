/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "TakeoffComplexItem.h"
#include "JsonHelper.h"
#include "MissionController.h"
#include "QGCGeo.h"
#include "SimpleMissionItem.h"
#include "PlanMasterController.h"
#include "FlightPathSegment.h"
#include "TakeoffMissionItem.h"

#include <QPolygonF>

QGC_LOGGING_CATEGORY(TakeoffComplexItemLog, "TakeoffComplexItemLog")
const char* TakeoffComplexItem::takeoffDistName                 = "takeoffDist";
const char* TakeoffComplexItem::vtolAltName                     = "vtolAlt";
const char* TakeoffComplexItem::climboutAltName                 = "climboutAlt";
const char* TakeoffComplexItem::takeoffHeadingName              = "takeoffHeading";
const char* TakeoffComplexItem::loiterClockwiseName             = "loiterClockwise";
const char* TakeoffComplexItem::useLoiterToAltName              = "useLoiterToAlt";
const char* TakeoffComplexItem::gradientName                    = "gradient";

//unsure what these do yet likely something mission download/upload -mwrighte38
const char* TakeoffComplexItem::_jsonVtolTakeoffCoordinateCoordinateKey = "vtolTakeoffCoordinate";
const char* TakeoffComplexItem::_jsonLoiterRadiusKey            = "loiterRadius";
const char* TakeoffComplexItem::_jsonLoiterClockwiseKey         = "loiterClockwise";
const char* TakeoffComplexItem::_jsonLandingCoordinateKey       = "landCoordinate";
const char* TakeoffComplexItem::_jsonAltitudesAreRelativeKey    = "altitudesAreRelative";
const char* TakeoffComplexItem::_jsonUseLoiterToAltKey          = "useLoiterToAlt";
const char* TakeoffComplexItem::_jsonStopTakingPhotosKey        = "stopTakingPhotos";
const char* TakeoffComplexItem::_jsonStopTakingVideoKey         = "stopVideoPhotos";

// Deprecated keys

// Support for separate relative alt settings for land/loiter was removed. It now only has a single
// relative alt setting stored in _jsonAltitudesAreRelativeKey.
const char* TakeoffComplexItem::_jsonDeprecatedLandingAltitudeRelativeKey   = "landAltitudeRelative";
const char* TakeoffComplexItem::_jsonDeprecatedLoiterAltitudeRelativeKey    = "loiterAltitudeRelative";

// Name changed from _jsonDeprecatedLoiterCoordinateKey to _jsonFinalApproachCoordinateKey to reflect
// the new support for using either a loiter or just a waypoint as the approach entry point.
const char* TakeoffComplexItem::_jsonDeprecatedLoiterCoordinateKey          = "loiterCoordinate";

TakeoffComplexItem::TakeoffComplexItem(PlanMasterController* masterController, bool flyView)
    : ComplexMissionItem        (masterController, flyView)
{
    _isIncomplete = false;

    // The following is used to compress multiple recalc calls in a row to into a single call.
    connect(this, &TakeoffComplexItem::_updateFlightPathSegmentsSignal, this, &TakeoffComplexItem::_updateFlightPathSegmentsDontCallDirectly,   Qt::QueuedConnection);
    qgcApp()->addCompressedSignal(QMetaMethod::fromSignal(&TakeoffComplexItem::_updateFlightPathSegmentsSignal));
}

void TakeoffComplexItem::_init(void)
{   

    connect(takeoffDist(),              &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromHeadingAndDistanceChange);
    connect(vtolAlt(),                  &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromCoordinateChange);
    connect(climboutAlt(),              &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromCoordinateChange);

    connect(takeoffHeading(),           &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromRadiusChange);
    connect(loiterClockwise(),          &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromRadiusChange);
    connect(useLoiterToAlt(),           &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromHeadingAndDistanceChange);
    connect(gradient(),                 &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromRadiusChange);

    connect(this,                       &TakeoffComplexItem::vtolTakeoffCoordinateChanged,  this, &TakeoffComplexItem::_recalcFromCoordinateChange);
    connect(this,                       &TakeoffComplexItem::climboutCoordinateChanged,     this, &TakeoffComplexItem::_recalcFromCoordinateChange);

    connect(vtolAlt(),                  &Fact::valueChanged,                                this, &TakeoffComplexItem::_setDirty);
    connect(climboutAlt(),              &Fact::valueChanged,                                this, &TakeoffComplexItem::_setDirty);
    connect(takeoffDist(),              &Fact::valueChanged,                                this, &TakeoffComplexItem::_setDirty);

    connect(this,                       &TakeoffComplexItem::vtolTakeoffCoordinateChanged,  this, &TakeoffComplexItem::_setDirty);
    connect(this,                       &TakeoffComplexItem::climboutCoordinateChanged,     this, &TakeoffComplexItem::_setDirty);

    connect(this,                       &TakeoffComplexItem::altitudesAreRelativeChanged,   this, &TakeoffComplexItem::_setDirty);
    connect(this,                       &TakeoffComplexItem::altitudesAreRelativeChanged,   this, &TakeoffComplexItem::_amslEntryAltChanged);
    connect(this,                       &TakeoffComplexItem::altitudesAreRelativeChanged,   this, &TakeoffComplexItem::_amslExitAltChanged);

    connect(vtolAlt(),                  &Fact::valueChanged,                                this, &TakeoffComplexItem::_amslEntryAltChanged);
    connect(climboutAlt(),              &Fact::valueChanged,                                this, &TakeoffComplexItem::_amslExitAltChanged);

    connect(this,                       &TakeoffComplexItem::amslEntryAltChanged,           this, &TakeoffComplexItem::maxAMSLAltitudeChanged);
    connect(this,                       &TakeoffComplexItem::amslExitAltChanged,            this, &TakeoffComplexItem::minAMSLAltitudeChanged);

    connect(this,                       &TakeoffComplexItem::takeoffCoordSetChanged,        this, &TakeoffComplexItem::readyForSaveStateChanged);
    connect(this,                       &TakeoffComplexItem::wizardModeChanged,             this, &TakeoffComplexItem::readyForSaveStateChanged);

    connect(this,                       &TakeoffComplexItem::vtolTakeoffCoordinateChanged,  this, &TakeoffComplexItem::complexDistanceChanged);
    connect(this,                       &TakeoffComplexItem::climboutCoordinateChanged,     this, &TakeoffComplexItem::complexDistanceChanged);

    connect(this,                       &TakeoffComplexItem::vtolTakeoffCoordinateChanged,  this, &TakeoffComplexItem::_updateFlightPathSegmentsSignal);
    connect(this,                       &TakeoffComplexItem::climboutCoordinateChanged,     this, &TakeoffComplexItem::_updateFlightPathSegmentsSignal);

    connect(vtolAlt(),                  &Fact::valueChanged,                                this, &TakeoffComplexItem::_updateFlightPathSegmentsSignal);
    connect(climboutAlt(),              &Fact::valueChanged,                                this, &TakeoffComplexItem::_updateFlightPathSegmentsSignal);
    connect(this,                       &TakeoffComplexItem::altitudesAreRelativeChanged,   this, &TakeoffComplexItem::_updateFlightPathSegmentsSignal);
    connect(_missionController,         &MissionController::plannedHomePositionChanged,     this, &TakeoffComplexItem::_updateFlightPathSegmentsSignal);

    connect(vtolAlt(),                  &Fact::valueChanged,                                this, &TakeoffComplexItem::_updateVtolTakeoffCoodinateAltitudeFromFact);
    connect(climboutAlt(),              &Fact::valueChanged,                                this, &TakeoffComplexItem::_updateClimboutCoordinateAltitudeFromFact);

}

double TakeoffComplexItem::complexDistance(void) const
{
    return vtolTakeoffCoordinate().distanceTo(climboutCoordinate());
}

void TakeoffComplexItem::setClimboutCoordinate(const QGeoCoordinate& coordinate)
{
    if (coordinate != _climboutCoordinate) {
        _climboutCoordinate = coordinate;
        if (_takeoffCoordSet) {
            emit exitCoordinateChanged(coordinate);
            emit climboutCoordinateChanged(coordinate);
        } else {
            _ignoreRecalcSignals = true;
            emit exitCoordinateChanged(coordinate);
            emit climboutCoordinateChanged(coordinate);
            _ignoreRecalcSignals = false;
            _takeoffCoordSet = true;
            _recalcFromHeadingAndDistanceChange();
            _vtolTakeoffCoordSet = true;
            emit takeoffCoordSetChanged(true);
        }
    }
}

void TakeoffComplexItem::setVtolTakeoffCoordinate(const QGeoCoordinate& coordinate)
{
    if (coordinate != _vtolTakeoffCoordinate) {
        _vtolTakeoffCoordinate = coordinate;
        emit coordinateChanged(coordinate);
        emit vtolTakeoffCoordinateChanged(coordinate);
    }
}

QPointF TakeoffComplexItem::_rotatePoint(const QPointF& point, const QPointF& origin, double angle)
{
    QPointF rotated;
    double radians = (M_PI / 180.0) * angle;

    rotated.setX(((point.x() - origin.x()) * cos(radians)) - ((point.y() - origin.y()) * sin(radians)) + origin.x());
    rotated.setY(((point.x() - origin.x()) * sin(radians)) + ((point.y() - origin.y()) * cos(radians)) + origin.y());

    return rotated;
}

void TakeoffComplexItem::_recalcFromHeadingAndDistanceChange(void)
{

    // Fixed:
    //      land
    //      heading
    //      distance
    //      radius
    // Adjusted:
    //      loiter
    //      loiter tangent
    //      glide slope

    if (!_ignoreRecalcSignals && _takeoffCoordSet) {

        Vehicle* vehicle = qgcApp()->toolbox()->multiVehicleManager()->activeVehicle();

        if (!vehicle){

        if(_vtolTakeoffCoordSet){
            takeoffHeading()->setRawValue(_climboutCoordinate.azimuthTo(_vtolTakeoffCoordinate));
        }

        // These are our known values
        double takeoffdistance = takeoffDist()->rawValue().toDouble();
        double heading = takeoffHeading()->rawValue().toDouble();

        if (useLoiterToAlt()->rawValue().toBool()){

            takeoffdistance = takeoffdistance + 100;
        }

       if(!_vtolTakeoffCoordSet){
        _vtolTakeoffCoordinate = _climboutCoordinate.atDistanceAndAzimuth(takeoffdistance, heading);
        }
       else{
           _climboutCoordinate = _vtolTakeoffCoordinate.atDistanceAndAzimuth(takeoffdistance, heading+180);
       }
        _ignoreRecalcSignals = true;
        emit vtolTakeoffCoordinateChanged(_vtolTakeoffCoordinate);
        emit climboutCoordinateChanged(_climboutCoordinate);
        emit coordinateChanged(_vtolTakeoffCoordinate);
        _ignoreRecalcSignals = false;
        }

        else if (vehicle){
            if (vehicle->homePosition().isValid() && (vehicle->homePosition().latitude() != 0 || vehicle->homePosition().longitude() != 0)){
            _vtolTakeoffCoordinate.setLatitude(vehicle->homePosition().latitude());
            _vtolTakeoffCoordinate.setLongitude(vehicle->homePosition().longitude());

            double takeoffdistance = takeoffDist()->rawValue().toDouble();
            double heading = _vtolTakeoffCoordinate.azimuthTo(_climboutCoordinate);

            _climboutCoordinate = _vtolTakeoffCoordinate.atDistanceAndAzimuth(takeoffdistance, heading);
            _ignoreRecalcSignals = true;
            emit vtolTakeoffCoordinateChanged(_vtolTakeoffCoordinate);
            emit climboutCoordinateChanged(_climboutCoordinate);
            emit coordinateChanged(_climboutCoordinate);
            _ignoreRecalcSignals = false;
            }
        }

    }

}

void TakeoffComplexItem::_recalcFromRadiusChange(void)
{

}

void TakeoffComplexItem::_recalcFromCoordinateChange(void)
{
    // These are our known values

    // _takeoffCoordSet = true;
        if(!_ignoreRecalcSignals && _takeoffCoordSet){

            Vehicle* vehicle = qgcApp()->toolbox()->multiVehicleManager()->activeVehicle();

            if (vehicle){
                if (vehicle->homePosition().isValid() && (vehicle->homePosition().latitude() != 0 || vehicle->homePosition().longitude() != 0)){
                _vtolTakeoffCoordinate.setLatitude(vehicle->homePosition().latitude());
                _vtolTakeoffCoordinate.setLongitude(vehicle->homePosition().longitude());
                }
            }

        double takeoffdist = _vtolTakeoffCoordinate.distanceTo(_climboutCoordinate);

        if (takeoffdist < takeoffDist()->rawMin().toDouble()){
            takeoffdist = takeoffDist()->rawMin().toDouble();
        }
        else if (takeoffdist > takeoffDist()->rawMax().toDouble()){
            takeoffdist = takeoffDist()->rawMax().toDouble();
        }
        double heading = _vtolTakeoffCoordinate.azimuthTo(_climboutCoordinate);


        // Heading is from loiter to land, hence +180
        _climboutCoordinate = _vtolTakeoffCoordinate.atDistanceAndAzimuth(takeoffdist, heading);

        _ignoreRecalcSignals = true;
        takeoffDist()->setRawValue(takeoffdist);
        emit climboutCoordinateChanged(_climboutCoordinate);
        emit vtolTakeoffCoordinateChanged(_vtolTakeoffCoordinate);
        emit coordinateChanged(_vtolTakeoffCoordinate);
        _ignoreRecalcSignals = false;
        }
}

int TakeoffComplexItem::lastSequenceNumber(void) const
{
    // Fixed items are:
    //  land start, loiter, land
    // Optional items are:
    //  stop photos/video
    return _sequenceNumber;
}

void TakeoffComplexItem::appendMissionItems(QList<MissionItem*>& items, QObject* missionItemParent)
{
    int seqNum = _sequenceNumber;

    // IMPORTANT NOTE: Any changes here must also be taken into account in scanForItem

    MissionItem* item = _createVtolTakeoffItem(seqNum++, missionItemParent);
    items.append(item);

    //create loiter item
    item = _createClimboutItem(seqNum++, missionItemParent);
    items.append(item);

}

MissionItem* TakeoffComplexItem::_createVtolTakeoffItem(int seqNum, QObject* parent)
{
    return new MissionItem(seqNum,                              // sequence number
                           MAV_CMD_NAV_VTOL_TAKEOFF,               // MAV_CMD
                           MAV_FRAME_MISSION,                   // MAV_FRAME
                           0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                           vtolAlt()->rawValue().toDouble(),   // param 1-7
                           true,                                // autoContinue
                           false,                               // isCurrentItem
                           parent);
}


//loiterRadius()->rawValue().toDouble() * (_loiterClockwise()->rawValue().toBool() ? 1.0 : -1.0),

MissionItem* TakeoffComplexItem::_createClimboutItem(int seqNum, QObject* parent)
{
    if (useLoiterToAlt()->rawValue().toBool()) {
        return new MissionItem(seqNum,
                               MAV_CMD_NAV_LOITER_TO_ALT,
                               _altitudesAreRelative ? MAV_FRAME_GLOBAL_RELATIVE_ALT : MAV_FRAME_GLOBAL,
                               1.0,             // Heading required = true
                               0.0,
                               0.0,             // param 3 - unused
                               1.0,             // Exit crosstrack - tangent of loiter to land point
                               _climboutCoordinate.latitude(),
                               _climboutCoordinate.longitude(),
                               climboutAlt()->rawValue().toFloat(),
                               true,            // autoContinue
                               false,           // isCurrentItem
                               parent);
    } else {
        return new MissionItem(seqNum,
                               MAV_CMD_NAV_WAYPOINT,
                               _altitudesAreRelative ? MAV_FRAME_GLOBAL_RELATIVE_ALT : MAV_FRAME_GLOBAL,
                               0,               // No hold time
                               0,               // Use default acceptance radius
                               0,               // Pass through waypoint
                               qQNaN(),         // Yaw not specified
                               _climboutCoordinate.latitude(),
                               _climboutCoordinate.longitude(),
                               climboutAlt()->rawValue().toFloat(),
                               true,            // autoContinue
                               false,           // isCurrentItem
                               parent);
    }
}

bool TakeoffComplexItem::_scanForItem(QmlObjectListModel* visualItems, bool flyView, PlanMasterController* masterController, IsTakeoffItemFunc IsTakeoffItemFunc, CreateItemFunc createItemFunc)
{
    qCDebug(TakeoffComplexItemLog) << "TakeoffComplexItem::scanForItem count" << visualItems->count();

    if (visualItems->count() < 3) {
        return false;
    }
    return false;
}

void TakeoffComplexItem::applyNewAltitude(double newAltitude)
{
    climboutAlt()->setRawValue(newAltitude);
}

TakeoffComplexItem::ReadyForSaveState TakeoffComplexItem::readyForSaveState(void) const
{
    return _takeoffCoordSet && !_wizardMode ? ReadyForSave : NotReadyForSaveData;
}

void TakeoffComplexItem::setDirty(bool dirty)
{
    if (_dirty != dirty) {
        _dirty = dirty;
        emit dirtyChanged(_dirty);
    }
}

void TakeoffComplexItem::_setDirty(void)
{
    setDirty(true);
}

void TakeoffComplexItem::setSequenceNumber(int sequenceNumber)
{
    if (_sequenceNumber != sequenceNumber) {
        _sequenceNumber = sequenceNumber;
        emit sequenceNumberChanged(sequenceNumber);
        emit lastSequenceNumberChanged(lastSequenceNumber());
    }
}

double TakeoffComplexItem::amslEntryAlt(void) const
{
    return vtolAlt()->rawValue().toDouble(); //remove now as altitude is relative for vtol wp + (_altitudesAreRelative ? _missionController->plannedHomePosition().altitude() : 0);
}

double TakeoffComplexItem::amslExitAlt(void) const
{
    return climboutAlt()->rawValue().toDouble() + (_altitudesAreRelative ? _missionController->plannedHomePosition().altitude() : 0);
}

void TakeoffComplexItem::_signalLastSequenceNumberChanged(void)
{
    emit lastSequenceNumberChanged(lastSequenceNumber());
}

void TakeoffComplexItem::_updateClimboutCoordinateAltitudeFromFact(void)
{
    _climboutCoordinate.setAltitude(climboutAlt()->rawValue().toDouble());
    emit climboutCoordinateChanged(_climboutCoordinate);
    emit coordinateChanged(_climboutCoordinate);
}

void TakeoffComplexItem::_updateVtolTakeoffCoodinateAltitudeFromFact(void)
{
    _vtolTakeoffCoordinate.setAltitude(vtolAlt()->rawValue().toDouble());
    emit vtolTakeoffCoordinateChanged(_vtolTakeoffCoordinate);
}

double TakeoffComplexItem::greatestDistanceTo(const QGeoCoordinate &other) const
{
    return qMax(_vtolTakeoffCoordinate.distanceTo(other),_climboutCoordinate.distanceTo(other));
}

QJsonObject TakeoffComplexItem::_save(void)
{
    QJsonObject saveObject;

    QGeoCoordinate coordinate;
    QJsonValue jsonCoordinate;

    coordinate = _vtolTakeoffCoordinate;
    coordinate.setAltitude(vtolAlt()->rawValue().toDouble());
    JsonHelper::saveGeoCoordinate(coordinate, true /* writeAltitude */, jsonCoordinate);
    saveObject[_jsonVtolTakeoffCoordinateCoordinateKey] = jsonCoordinate;

    coordinate = _climboutCoordinate;
    coordinate.setAltitude(climboutAlt()->rawValue().toDouble());
    JsonHelper::saveGeoCoordinate(coordinate, true /* writeAltitude */, jsonCoordinate);
    saveObject[_jsonLandingCoordinateKey] = jsonCoordinate;

    saveObject[_jsonLoiterClockwiseKey]         = loiterClockwise()->rawValue().toBool();
    saveObject[_jsonUseLoiterToAltKey]          = useLoiterToAlt()->rawValue().toBool();
    saveObject[_jsonAltitudesAreRelativeKey]    = _altitudesAreRelative;

    return saveObject;
}

bool TakeoffComplexItem::_load(const QJsonObject& complexObject, int sequenceNumber, const QString& jsonComplexItemTypeValue, bool useDeprecatedRelAltKeys, QString& errorString)
{
    return false;
}

void TakeoffComplexItem::setAltitudesAreRelative(bool altitudesAreRelative)
{
    if (altitudesAreRelative != _altitudesAreRelative) {
        _altitudesAreRelative = altitudesAreRelative;
        emit altitudesAreRelativeChanged(_altitudesAreRelative);
    }
}
