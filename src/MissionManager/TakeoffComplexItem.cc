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
const char* TakeoffComplexItem::loiterRadiusName                = "LoiterRadius";
const char* TakeoffComplexItem::transitionDistanceName          = "transitionDistance";


const char* TakeoffComplexItem::_jsonVtolTakeoffCoordinateCoordinateKey = "vtolTakeoffCoordinate";
const char* TakeoffComplexItem::_jsonClimboutCoordinateKey       = "climboutCoordinate";
const char* TakeoffComplexItem::_jsonLoiterRadiusKey            = "loiterRadius";
const char* TakeoffComplexItem::_jsonLoiterClockwiseKey         = "loiterClockwise";
const char* TakeoffComplexItem::_jsonUseLoiterToAltKey          = "useLoiterToAlt";


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
    connect(loiterRadius(),             &Fact::valueChanged,                                this, &TakeoffComplexItem::_recalcFromCoordinateChange);

    connect(takeoffDist(),              &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromHeadingAndDistanceChange);
    connect(vtolAlt(),                  &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromCoordinateChange);
    connect(climboutAlt(),              &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromCoordinateChange);

    connect(takeoffHeading(),           &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromRadiusChange);
    connect(loiterClockwise(),          &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromRadiusChange);
    connect(useLoiterToAlt(),           &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromCoordinateChange);
    connect(gradient(),                 &Fact::rawValueChanged,                             this, &TakeoffComplexItem::_recalcFromRadiusChange);

    connect(this,                       &TakeoffComplexItem::vtolTakeoffCoordinateChanged,  this, &TakeoffComplexItem::_recalcFromCoordinateChange);
    connect(this,                       &TakeoffComplexItem::climboutCoordinateChanged,     this, &TakeoffComplexItem::_recalcFromCoordinateChange);

    connect(loiterRadius(),             &Fact::valueChanged,                                this, &TakeoffComplexItem::_setDirty);
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
        //emit exitCoordinateChanged(_climboutCoordinate);
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
        double minDist = useLoiterToAlt()->rawValue().toBool() ? takeoffDist()->rawMin().toDouble() + loiterRadius()->rawValue().toDouble() : takeoffDist()->rawMin().toDouble();

        if (takeoffdistance < minDist){
            takeoffdistance = minDist;
        }
        else if (takeoffdistance > takeoffDist()->rawMax().toDouble()){
            takeoffdistance = takeoffDist()->rawMax().toDouble();
        }

       if(!_vtolTakeoffCoordSet){
        setVtolTakeoffCoordinate(_climboutCoordinate.atDistanceAndAzimuth(takeoffdistance, heading));
        }
       else{
           setClimboutCoordinate(_vtolTakeoffCoordinate.atDistanceAndAzimuth(takeoffdistance, heading+180));
       }
        _ignoreRecalcSignals = true;
        takeoffDist()->setRawValue(takeoffdistance);
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

            setClimboutCoordinate(_vtolTakeoffCoordinate.atDistanceAndAzimuth(takeoffdistance, heading));
            _ignoreRecalcSignals = true;
            takeoffDist()->setRawValue(takeoffdistance);
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
        double minDist = useLoiterToAlt()->rawValue().toBool() ? takeoffDist()->rawMin().toDouble() + loiterRadius()->rawValue().toDouble() : takeoffDist()->rawMin().toDouble();

        if (takeoffdist < minDist){
            takeoffdist = minDist;
        }
        else if (takeoffdist > takeoffDist()->rawMax().toDouble()){
            takeoffdist = takeoffDist()->rawMax().toDouble();
        }
        double heading = _vtolTakeoffCoordinate.azimuthTo(_climboutCoordinate);


        // Heading is from loiter to land, hence +180
        setClimboutCoordinate(_vtolTakeoffCoordinate.atDistanceAndAzimuth(takeoffdist, heading));

        _ignoreRecalcSignals = true;
        takeoffDist()->setRawValue(takeoffdist);
        emit vtolTakeoffCoordinateChanged(_vtolTakeoffCoordinate);
        emit climboutCoordinateChanged(_climboutCoordinate);
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

    int _sequenceNumber = 2;

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
                           _altitudesAreRelative ? MAV_FRAME_GLOBAL_RELATIVE_ALT : MAV_FRAME_GLOBAL,
                           0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                           vtolAlt()->rawValue().toDouble(),   // param 1-7
                           true,                                // autoContinue
                           false,                               // isCurrentItem
                           parent);
}

MissionItem* TakeoffComplexItem::_createClimboutItem(int seqNum, QObject* parent)
{
    if (useLoiterToAlt()->rawValue().toBool()) {
        return new MissionItem(seqNum,
                               MAV_CMD_NAV_LOITER_TO_ALT,
                               _altitudesAreRelative ? MAV_FRAME_GLOBAL_RELATIVE_ALT : MAV_FRAME_GLOBAL,
                               1.0,             // Heading required = true
                               loiterRadius()->rawValue().toDouble() * (_loiterClockwise()->rawValue().toBool() ? 1.0 : -1.0),
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

    if (visualItems->count() < 2) {
        return false;
    }

    // A valid takeoff pattern is comprised of the follow commands in this order at the end of the item list:
    //  VTOL Takeoff - required
    //  MAV_CMD_NAV_LOITER_TO_ALT or MAV_CMD_NAV_WAYPOINT

    // Start looking for the commands in beginning of list.
    int scanIndex = 0;

    if (scanIndex > visualItems->count()) {
        return false;
    }

    SimpleMissionItem* item = visualItems->value<SimpleMissionItem*>(scanIndex++);
    if (!item) {
        return false;
    }

    MissionItem& missionItemTakeoff = item->missionItem();
    if (!IsTakeoffItemFunc(missionItemTakeoff)) {
        return false;
    }

    MAV_FRAME landPointFrame = missionItemTakeoff.frame();

    if (scanIndex < 0 || scanIndex > visualItems->count() - 1) {
        return false;
    }

    item = visualItems->value<SimpleMissionItem*>(scanIndex++);
    if (!item) {
        return false;
    }

    bool useLoiterToAlt = true;
    MissionItem& missionItemClimbout = item->missionItem();
    if (missionItemClimbout.command() == MAV_CMD_NAV_LOITER_TO_ALT) {
        if (missionItemClimbout.frame() != landPointFrame ||
                missionItemClimbout.param1() != 1.0 || missionItemClimbout.param3() != 0 || missionItemClimbout.param4() != 1.0) {
            return false;
        }
    } else if (missionItemClimbout.command() == MAV_CMD_NAV_WAYPOINT) {
        if (missionItemClimbout.frame() != landPointFrame ||
                missionItemClimbout.param1() != 0 || missionItemClimbout.param2() != 0 || missionItemClimbout.param3() != 0 ||
                !qIsNaN(missionItemClimbout.param4()) ||
                qIsNaN(missionItemClimbout.param5()) || qIsNaN(missionItemClimbout.param6()) || qIsNaN(missionItemClimbout.param6())) {
            return false;
        }
        useLoiterToAlt = false;
    } else {
        return false;
    }

    // We made it this far so we do have a VTOL takeoff Pattern item at the end of the mission.
    // Since we have scanned it we need to remove the items for it fromt the list


    // Now stuff all the scanned information into the item

    TakeoffComplexItem* complexItem = createItemFunc(masterController, flyView);

    complexItem->_ignoreRecalcSignals = true;

    complexItem->_altitudesAreRelative = landPointFrame == MAV_FRAME_GLOBAL_RELATIVE_ALT;
    complexItem->setClimboutCoordinate(QGeoCoordinate(missionItemClimbout.param5(), missionItemClimbout.param6()));
    complexItem->climboutAlt()->setRawValue(missionItemClimbout.param7());
    complexItem->useLoiterToAlt()->setRawValue(useLoiterToAlt);

    if (useLoiterToAlt) {
        complexItem->loiterRadius()->setRawValue(qAbs(missionItemClimbout.param2()));
        complexItem->loiterClockwise()->setRawValue(missionItemClimbout.param2() > 0);
    }

    complexItem->_vtolTakeoffCoordinate.setLatitude(missionItemTakeoff.param5());
    complexItem->_vtolTakeoffCoordinate.setLongitude(missionItemTakeoff.param6());
    complexItem->vtolAlt()->setRawValue(missionItemTakeoff.param7());

    complexItem->_takeoffCoordSet = true;

    complexItem->_ignoreRecalcSignals = false;

    complexItem->_recalcFromCoordinateChange();
    complexItem->setDirty(false);

    visualItems->append(complexItem);

    return true;
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
    return vtolAlt()->rawValue().toDouble() + (_altitudesAreRelative ? _missionController->plannedHomePosition().altitude() : 0);
}

double TakeoffComplexItem::amslgroundAlt(void)
{
    return vtolAlt()->rawValue().toDouble() + (_altitudesAreRelative ? _missionController->plannedHomePosition().altitude() : 0);
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

    saveObject[_jsonClimboutCoordinateKey] = jsonCoordinate;
    saveObject[_jsonUseLoiterToAltKey]          = useLoiterToAlt()->rawValue().toBool();
    saveObject[_jsonLoiterRadiusKey]            = loiterRadius()->rawValue().toDouble();
    saveObject[_jsonLoiterClockwiseKey]         = loiterClockwise()->rawValue().toBool();


    return saveObject;
}

bool TakeoffComplexItem::_load(const QJsonObject& complexObject, int sequenceNumber, const QString& jsonComplexItemTypeValue, bool useDeprecatedRelAltKeys, QString& errorString)
{
    QList<JsonHelper::KeyValidateInfo> keyInfoList = {
        { JsonHelper::jsonVersionKey,                   QJsonValue::Double, true },
        { VisualMissionItem::jsonTypeKey,               QJsonValue::String, true },
        { ComplexMissionItem::jsonComplexItemTypeKey,   QJsonValue::String, true },
        { _jsonVtolTakeoffCoordinateCoordinateKey,      QJsonValue::Array,  true },
        { _jsonLoiterRadiusKey,                         QJsonValue::Double, true },
        { _jsonLoiterClockwiseKey,                      QJsonValue::Bool,   true },
        { _jsonClimboutCoordinateKey,                   QJsonValue::Array,  true },
        { _jsonUseLoiterToAltKey,                       QJsonValue::Bool,   true },
    };

    if (!JsonHelper::validateKeys(complexObject, keyInfoList, errorString)) {
        return false;
    }

    QString itemType = complexObject[VisualMissionItem::jsonTypeKey].toString();
    QString complexType = complexObject[ComplexMissionItem::jsonComplexItemTypeKey].toString();
    if (itemType != VisualMissionItem::jsonTypeComplexItemValue || complexType != jsonComplexItemTypeValue) {
        errorString = tr("%1 does not support loading this complex mission item type: %2:%3").arg(qgcApp()->applicationName()).arg(itemType).arg(complexType);
        return false;
    }

    setSequenceNumber(sequenceNumber);

    _ignoreRecalcSignals = true;

    QGeoCoordinate coordinate;
    if (!JsonHelper::loadGeoCoordinate(complexObject[_jsonVtolTakeoffCoordinateCoordinateKey], true /* altitudeRequired */, coordinate, errorString)) {
        return false;
    }
    _vtolTakeoffCoordinate = coordinate;

    vtolAlt()->setRawValue(coordinate.altitude());

    if (!JsonHelper::loadGeoCoordinate(complexObject[_jsonClimboutCoordinateKey], true /* altitudeRequired */, coordinate, errorString)) {
        return false;
    }
    _climboutCoordinate = coordinate;
    climboutAlt()->setRawValue(coordinate.altitude());

    loiterRadius()->setRawValue(complexObject[_jsonLoiterRadiusKey].toDouble());
    loiterClockwise()->setRawValue(complexObject[_jsonLoiterClockwiseKey].toBool());
    useLoiterToAlt()->setRawValue(complexObject[_jsonUseLoiterToAltKey].toBool(true));

    _takeoffCoordSet        = true;
    _ignoreRecalcSignals    = false;

    _recalcFromCoordinateChange();
    emit coordinateChanged(this->coordinate());    // This will kick off terrain query

    return true;
}

void TakeoffComplexItem::setAltitudesAreRelative(bool altitudesAreRelative)
{
    if (altitudesAreRelative != _altitudesAreRelative) {
        _altitudesAreRelative = altitudesAreRelative;
        emit altitudesAreRelativeChanged(_altitudesAreRelative);
    }
}
