/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VTOLTakeoffComplexItem.h"
#include "JsonHelper.h"
#include "MissionController.h"
#include "QGCGeo.h"
#include "SimpleMissionItem.h"
#include "PlanMasterController.h"
#include "FlightPathSegment.h"
#include "QGC.h"

#include <QPolygonF>

QGC_LOGGING_CATEGORY(VTOLTakeoffComplexItemLog, "VTOLTakeoffComplexItemLog")

const QString VTOLTakeoffComplexItem::name(VTOLTakeoffComplexItem::tr("VTOL Takeoff"));

const char* VTOLTakeoffComplexItem::settingsGroup =            "VTOLTakeoff";
const char* VTOLTakeoffComplexItem::jsonComplexItemTypeValue = "VTOLTakeoffPattern";

VTOLTakeoffComplexItem::VTOLTakeoffComplexItem(PlanMasterController* masterController, bool flyView)
    : TakeoffComplexItem        (masterController, flyView)
    , _metaDataMap              (FactMetaData::createMapFromJsonFile(QStringLiteral(":/json/VTOLTakeoffPattern.FactMetaData.json"), this))
    , _takeoffDistFact          (settingsGroup, _metaDataMap[takeoffDistName])
    , _vtolAltFact              (settingsGroup, _metaDataMap[vtolAltName])
    , _climboutAltFact          (settingsGroup, _metaDataMap[climboutAltName])
    , _takeoffHeadingFact       (settingsGroup, _metaDataMap[takeoffHeadingName])
    , _loiterClockwiseFact      (settingsGroup, _metaDataMap[loiterClockwiseName])
    , _useLoiterToAltFact       (settingsGroup, _metaDataMap[useLoiterToAltName])
    , _gradientFact             (settingsGroup, _metaDataMap[gradientName])
{
    _editorQml      = "qrc:/qml/VTOLTakeoffPatternEditor.qml";
    _isIncomplete   = false;

    _init();

    setDirty(false);
}

void VTOLTakeoffComplexItem::save(QJsonArray&  missionItems)
{
    QJsonObject saveObject = _save();

    saveObject[JsonHelper::jsonVersionKey] =                    1;
    saveObject[VisualMissionItem::jsonTypeKey] =                VisualMissionItem::jsonTypeComplexItemValue;
    saveObject[ComplexMissionItem::jsonComplexItemTypeKey] =    jsonComplexItemTypeValue;

    missionItems.append(saveObject);
}

bool VTOLTakeoffComplexItem::load(const QJsonObject& complexObject, int sequenceNumber, QString& errorString)
{
    QList<JsonHelper::KeyValidateInfo> keyInfoList = {
        { JsonHelper::jsonVersionKey, QJsonValue::Double, true },
    };
    if (!JsonHelper::validateKeys(complexObject, keyInfoList, errorString)) {
        return false;
    }

    int version = complexObject[JsonHelper::jsonVersionKey].toInt();
    if (version != 1) {
        errorString = tr("%1 complex item version %2 not supported").arg(jsonComplexItemTypeValue).arg(version);
        _ignoreRecalcSignals = false;
        return false;
    }

    return _load(complexObject, sequenceNumber, jsonComplexItemTypeValue, false /* useDeprecatedRelAltKeys */, errorString);
}

MissionItem* VTOLTakeoffComplexItem::_createTakeoffItem(int seqNum, bool altRel, double lat, double lon, double alt, QObject* parent)
{
    return new MissionItem(seqNum,
                           MAV_CMD_NAV_VTOL_LAND,
                           altRel ? MAV_FRAME_GLOBAL_RELATIVE_ALT : MAV_FRAME_GLOBAL,
                           0.0, 0.0, 0.0,
                           qQNaN(),         // Yaw - not specified
                           lat, lon, alt,
                           true,            // autoContinue
                           false,           // isCurrentItem
                           parent);

}

bool VTOLTakeoffComplexItem::_isValidLandItem(const MissionItem& missionItem)
{
    if (missionItem.command() != MAV_CMD_NAV_LAND ||
            !(missionItem.frame() == MAV_FRAME_GLOBAL_RELATIVE_ALT || missionItem.frame() == MAV_FRAME_GLOBAL) ||
            missionItem.param1() != 0 || missionItem.param2() != 0 || missionItem.param3() != 0 || !qIsNaN(missionItem.param4())) {
        return false;
    } else {
        return true;
    }
}

bool VTOLTakeoffComplexItem::scanForItem(QmlObjectListModel* visualItems, bool flyView, PlanMasterController* masterController)
{
    return _scanForItem(visualItems, flyView, masterController, _isValidLandItem, _createItem);
}

// Never call this method directly. If you want to update the flight segments you emit _updateFlightPathSegmentsSignal()
void VTOLTakeoffComplexItem::_updateFlightPathSegmentsDontCallDirectly(void)
{
    if (_cTerrainCollisionSegments != 0) {
        _cTerrainCollisionSegments = 0;
        emit terrainCollisionChanged(false);
    }

    _flightPathSegments.beginReset();
    _flightPathSegments.clearAndDeleteContents();

    _appendFlightPathSegment(FlightPathSegment::SegmentTypeTakeoff, vtolTakeoffCoordinate(), amslEntryAlt(), vtolTakeoffCoordinate(), amslExitAlt());

    _appendFlightPathSegment(FlightPathSegment::SegmentTypeGeneric, climboutCoordinate(), amslEntryAlt(), climboutCoordinate(), amslExitAlt());

    _flightPathSegments.endReset();

    if (_cTerrainCollisionSegments != 0) {
        emit terrainCollisionChanged(true);
    }

    _masterController->missionController()->recalcTerrainProfile();
}
