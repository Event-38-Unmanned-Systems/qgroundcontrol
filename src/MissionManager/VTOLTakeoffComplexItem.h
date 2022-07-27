/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "TakeoffComplexItem.h"
#include "MissionItem.h"
#include "Fact.h"
#include "QGCLoggingCategory.h"

Q_DECLARE_LOGGING_CATEGORY(VTOLTakeoffComplexItemLog)

class VTOLTakeoffPatternTest;
class PlanMasterController;

class VTOLTakeoffComplexItem : public TakeoffComplexItem
{
    Q_OBJECT

public:

    VTOLTakeoffComplexItem(PlanMasterController* masterController, bool flyView);

    /// Scans the loaded items for a landing pattern complex item
    static bool scanForItem(QmlObjectListModel* visualItems, bool flyView, PlanMasterController* masterController);

    // Overrides from ComplexMissionItem
    QString patternName         (void) const final { return name; }
    bool    load                (const QJsonObject& complexObject, int sequenceNumber, QString& errorString) final;
    QString mapVisualQML        (void) const final { return QStringLiteral("VTOLTakeoffPatternMapVisual.qml"); }

    // Overrides from VisualMissionItem
    void                save                        (QJsonArray&  missionItems) final;

    static const QString name;

    static const char* jsonComplexItemTypeValue;

    static const char* settingsGroup;


private slots:
    void _updateFlightPathSegmentsDontCallDirectly(void) override;

private:
    static TakeoffComplexItem*  _createItem     (PlanMasterController* masterController, bool flyView) { return new VTOLTakeoffComplexItem(masterController, flyView); }
    static bool                 _isValidLandItem(const MissionItem& missionItem);

    // Overrides from LandingComplexItem
    const Fact*     _takeoffDist  (void) const final { return &_takeoffDistFact; }
    const Fact*     _vtolAlt              (void) const final { return &_vtolAltFact; }
    const Fact*     _climboutAlt  (void) const final { return &_climboutAltFact; }
    const Fact*     _takeoffHeading           (void) const final { return &_takeoffHeadingFact; }
    const Fact*     _loiterClockwise        (void) const final { return &_loiterClockwiseFact; }
    const Fact*     _useLoiterToAlt        (void) const final { return &_useLoiterToAltFact; }
    const Fact*     _gradient        (void) const final { return &_gradientFact;}
    const Fact*     _loiterRadius           (void) const final { return &_loiterRadiusFact; }

    MissionItem*    _createTakeoffItem         (int seqNum, bool altRel, double lat, double lon, double alt, QObject* parent) final;

    QMap<QString, FactMetaData*> _metaDataMap;
    Fact            _takeoffDistFact;
    Fact            _vtolAltFact;
    Fact            _climboutAltFact;
    Fact            _takeoffHeadingFact;
    Fact            _loiterClockwiseFact;
    Fact            _useLoiterToAltFact;
    Fact            _gradientFact;
    Fact            _loiterRadiusFact;

    friend VTOLTakeoffPatternTest;
};
