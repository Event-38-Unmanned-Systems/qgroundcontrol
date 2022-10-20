/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VTOLLandingComplexItem.h"
#include "JsonHelper.h"
#include "MissionController.h"
#include "QGCGeo.h"
#include "SimpleMissionItem.h"
#include "PlanMasterController.h"
#include "FlightPathSegment.h"
#include "QGC.h"

#include <QPolygonF>

QGC_LOGGING_CATEGORY(VTOLLandingComplexItemLog, "VTOLLandingComplexItemLog")

const QString VTOLLandingComplexItem::name(VTOLLandingComplexItem::tr("Landing Pattern"));

const char* VTOLLandingComplexItem::settingsGroup =            "VTOLLanding";
const char* VTOLLandingComplexItem::jsonComplexItemTypeValue = "vtolLandingPattern";
const char* VTOLLandingComplexItem::glideSlopeName      = "GlideSlope";

VTOLLandingComplexItem::VTOLLandingComplexItem(PlanMasterController* masterController, bool flyView)
    : LandingComplexItem        (masterController, flyView)
    , _metaDataMap              (FactMetaData::createMapFromJsonFile(QStringLiteral(":/json/VTOLLandingPattern.FactMetaData.json"), this))
    , _transitionAltFact        (settingsGroup, _metaDataMap[transitionAltName])
    , _transitionDistanceFact   (settingsGroup, _metaDataMap[transitionDistanceName])
    , _landingDistanceFact      (settingsGroup, _metaDataMap[finalApproachToLandDistanceName])
    , _finalApproachAltitudeFact(settingsGroup, _metaDataMap[finalApproachAltitudeName])
    , _loiterRadiusFact         (settingsGroup, _metaDataMap[loiterRadiusName])
    , _loiterClockwiseFact      (settingsGroup, _metaDataMap[loiterClockwiseName])
    , _landingHeadingFact       (settingsGroup, _metaDataMap[landingHeadingName])
    , _landingAltitudeFact      (settingsGroup, _metaDataMap[landingAltitudeName])
    , _useLoiterToAltFact       (settingsGroup, _metaDataMap[useLoiterToAltName])
    , _stopTakingPhotosFact     (settingsGroup, _metaDataMap[stopTakingPhotosName])
    , _stopTakingVideoFact      (settingsGroup, _metaDataMap[stopTakingVideoName])
    , _glideSlopeFact           (settingsGroup, _metaDataMap[glideSlopeName])
    , _terrainApproachFact      (settingsGroup, _metaDataMap[terrainApproachName])

{
    _editorQml      = "qrc:/qml/VTOLLandingPatternEditor.qml";
    _isIncomplete   = false;

    _init();

    connect(&_glideSlopeFact,           &Fact::valueChanged, this, &VTOLLandingComplexItem::_glideSlopeChanged);

    _recalcFromHeadingAndDistanceChange();

    setDirty(false);
}

void VTOLLandingComplexItem::save(QJsonArray&  missionItems)
{
    QJsonObject saveObject = _save();

    saveObject[JsonHelper::jsonVersionKey] =                    1;
    saveObject[VisualMissionItem::jsonTypeKey] =                VisualMissionItem::jsonTypeComplexItemValue;
    saveObject[ComplexMissionItem::jsonComplexItemTypeKey] =    jsonComplexItemTypeValue;

    missionItems.append(saveObject);
}

bool VTOLLandingComplexItem::load(const QJsonObject& complexObject, int sequenceNumber, QString& errorString)
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

MissionItem* VTOLLandingComplexItem::_createLandItem(int seqNum, bool altRel, double lat, double lon, double alt, QObject* parent)
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

void VTOLLandingComplexItem::_glideSlopeChanged(void)
{
    if (!_ignoreRecalcSignals) {
        double landingAltDifference = _finalApproachAltitudeFact.rawValue().toDouble() - _transitionAltFact.rawValue().toDouble();
        double glideSlope = _glideSlopeFact.rawValue().toDouble();
        _landingDistanceFact.setRawValue(landingAltDifference / qTan(qDegreesToRadians(glideSlope)) + _transitionDistanceFact.rawValue().toDouble());
    }
}

void VTOLLandingComplexItem::_calcGlideSlope(void)
{
    double landingAltDifference = _finalApproachAltitudeFact.rawValue().toDouble() - _transitionAltFact.rawValue().toDouble();
    double landingDistance = _landingDistanceFact.rawValue().toDouble() - _transitionDistanceFact.rawValue().toDouble();
    double slope = qRadiansToDegrees(qAtan(landingAltDifference / landingDistance));

    if (slope <= _glideSlopeFact.rawMin().toDouble()){
        _ignoreRecalcSignals = false;
       slope = _glideSlopeFact.rawMin().toDouble()+.01;
       _glideSlopeFact.setRawValue(slope);
       _ignoreRecalcSignals = true;
    }
    else if(slope >= _glideSlopeFact.rawMax().toDouble()){
        _ignoreRecalcSignals = false;
        slope = _glideSlopeFact.rawMax().toDouble()-.01;
        _glideSlopeFact.setRawValue(slope);
        _ignoreRecalcSignals = true;
     }
    else{
    _glideSlopeFact.setRawValue(slope);
    }
}

bool VTOLLandingComplexItem::_isValidLandItem(const MissionItem& missionItem)
{
    if (missionItem.command() != MAV_CMD_NAV_LAND ||
            !(missionItem.frame() == MAV_FRAME_GLOBAL_RELATIVE_ALT || missionItem.frame() == MAV_FRAME_GLOBAL) ||
            missionItem.param1() != 0 || missionItem.param2() != 0 || missionItem.param3() != 0 || !qIsNaN(missionItem.param4())) {
        return false;
    } else {
        return true;
    }
}

bool VTOLLandingComplexItem::scanForItem(QmlObjectListModel* visualItems, bool flyView, PlanMasterController* masterController)
{
    return _scanForItem(visualItems, flyView, masterController, _isValidLandItem, _createItem);
}

// Never call this method directly. If you want to update the flight segments you emit _updateFlightPathSegmentsSignal()
void VTOLLandingComplexItem::_updateFlightPathSegmentsDontCallDirectly(void)
{
    if (_cTerrainCollisionSegments != 0) {
        _cTerrainCollisionSegments = 0;
        emit terrainCollisionChanged(false);
    }

    _flightPathSegments.beginReset();
    _flightPathSegments.clearAndDeleteContents();
    _entrycoord = finalApproachCoordinate();
    _entrycoord.setAltitude(amslEntryAlt());

    if (useLoiterToAlt()->rawValue().toBool()) {
        if (_landingAltMode == QGroundControlQmlGlobal::AltitudeModeTerrainFrame){
            //_appendFlightPathSegment(FlightPathSegment::SegmentTypeGeneric, _entrycoord, amslEntryAlt(), _entrycoord,  amslLoiterAlt()); // Best we can do to simulate loiter circle terrain profile
        }
        _appendFlightPathSegment(FlightPathSegment::SegmentTypeGeneric, _entrycoord, amslEntryAlt(), loiterTangentCoordinate(),  amslLoiterAlt()); // Best we can do to simulate loiter circle terrain profile
        _appendFlightPathSegment(FlightPathSegment::SegmentTypeGeneric, loiterTangentCoordinate(), amslLoiterAlt(), transitionCoordinate(),        amslTransitionAlt());
    }
    else {
        _appendFlightPathSegment(FlightPathSegment::SegmentTypeGeneric, _entrycoord, amslEntryAlt(), transitionCoordinate(),        amslTransitionAlt());
    }

    _appendFlightPathSegment(FlightPathSegment::SegmentTypeGeneric, transitionCoordinate(), amslTransitionAlt(), landingCoordinate(), amslTransitionAlt());

    _appendFlightPathSegment(FlightPathSegment::SegmentTypeLand, landingCoordinate(), amslTransitionAlt(), landingCoordinate(), amslExitAlt());
    _flightPathSegments.endReset();

    if (_cTerrainCollisionSegments != 0) {
        emit terrainCollisionChanged(true);
    }

    emit coordinateChanged(coordinate());
    _missionController->_recalcFlightPathSegmentsSignal();
    _masterController->missionController()->recalcTerrainProfile();
}
