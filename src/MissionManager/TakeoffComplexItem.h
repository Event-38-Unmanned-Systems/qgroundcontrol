/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "ComplexMissionItem.h"
#include "MissionItem.h"
#include "Fact.h"
#include "QGCLoggingCategory.h"

Q_DECLARE_LOGGING_CATEGORY(TakeoffComplexItemLog)

class PlanMasterController;
class TakeoffComplexItemTest;

// Base class for landing patterns complex items.
class TakeoffComplexItem : public ComplexMissionItem
{
    Q_OBJECT

public:
    TakeoffComplexItem(PlanMasterController* masterController, bool flyView);
    Q_PROPERTY(Fact*            takeoffDist             READ    takeoffDist                                                     CONSTANT)
    Q_PROPERTY(Fact*            vtolAlt                 READ    vtolAlt                                                         CONSTANT)
    Q_PROPERTY(Fact*            loiterRadius            READ    loiterRadius                                                    CONSTANT)
    Q_PROPERTY(Fact*            climboutAlt             READ    climboutAlt                                                     CONSTANT)
    Q_PROPERTY(Fact*            takeoffHeading          READ    takeoffHeading                                                  CONSTANT)
    Q_PROPERTY(Fact*            loiterClockwise         READ    loiterClockwise                                                 CONSTANT)
    Q_PROPERTY(Fact*            useLoiterToAlt          READ    useLoiterToAlt                                                  CONSTANT)
    Q_PROPERTY(Fact*            gradient                READ    gradient                                                        CONSTANT)
    Q_PROPERTY(QGeoCoordinate   vtolTakeoffCoordinate   READ    vtolTakeoffCoordinate       WRITE setVtolTakeoffCoordinate      NOTIFY vtolTakeoffCoordinateChanged)
    Q_PROPERTY(QGeoCoordinate   climboutCoordinate      READ    climboutCoordinate          WRITE setClimboutCoordinate         NOTIFY climboutCoordinateChanged)
    Q_PROPERTY(bool             altitudesAreRelative    READ    altitudesAreRelative        WRITE setAltitudesAreRelative       NOTIFY altitudesAreRelativeChanged)
    Q_PROPERTY(bool             takeoffCoordSet         READ    takeoffCoordSet                                                 NOTIFY takeoffCoordSetChanged)

    const Fact* loiterRadius            (void) const { return _loiterRadius(); }
    const Fact* takeoffDist           (void) const { return _takeoffDist(); }
    const Fact* vtolAlt           (void) const { return _vtolAlt(); }
    const Fact* climboutAlt   (void) const { return _climboutAlt(); }
    const Fact* takeoffHeading            (void) const { return _takeoffHeading(); }
    const Fact* loiterClockwise         (void) const { return _loiterClockwise(); }
    const Fact* useLoiterToAlt          (void) const { return _useLoiterToAlt(); }
    const Fact* gradient        (void) const { return _gradient(); }

    Fact* takeoffDist         (void) { return const_cast<Fact*>(const_cast<const TakeoffComplexItem*>(this)->_takeoffDist()); };
    Fact* vtolAlt         (void) { return const_cast<Fact*>(const_cast<const TakeoffComplexItem*>(this)->_vtolAlt()); };
    Fact* climboutAlt (void) { return const_cast<Fact*>(const_cast<const TakeoffComplexItem*>(this)->_climboutAlt()); };
    Fact* takeoffHeading          (void) { return const_cast<Fact*>(const_cast<const TakeoffComplexItem*>(this)->_takeoffHeading()); };
    Fact* loiterClockwise       (void) { return const_cast<Fact*>(const_cast<const TakeoffComplexItem*>(this)->_loiterClockwise()); };
    Fact* useLoiterToAlt        (void) { return const_cast<Fact*>(const_cast<const TakeoffComplexItem*>(this)->_useLoiterToAlt()); };
    Fact* gradient      (void) { return const_cast<Fact*>(const_cast<const TakeoffComplexItem*>(this)->_gradient()); };
    Fact* loiterRadius          (void) { return const_cast<Fact*>(const_cast<const TakeoffComplexItem*>(this)->_loiterRadius()); };

    bool            altitudesAreRelative    (void) const { return _altitudesAreRelative; }
    bool            takeoffCoordSet         (void) const { return _takeoffCoordSet; }
    bool            vtolTakeoffCoordSet     (void) const { return _vtolTakeoffCoordSet; }

    QGeoCoordinate  climboutCoordinate       (void) const { return _climboutCoordinate; }
    QGeoCoordinate  vtolTakeoffCoordinate       (void) const { return _vtolTakeoffCoordinate; }

    void setVtolTakeoffCoordinate   (const QGeoCoordinate& coordinate);
    void setClimboutCoordinate    (const QGeoCoordinate& coordinate);
    void setAltitudesAreRelative    (bool altitudesAreRelative);

    // Overrides from ComplexMissionItem
    double  complexDistance     (void) const final;
    double  greatestDistanceTo  (const QGeoCoordinate &other) const final;
    int     lastSequenceNumber  (void) const final;

    // Overrides from VisualMissionItem
    bool                dirty                       (void) const final { return _dirty; }
    bool                isSimpleItem                (void) const final { return false; }
    bool                isStandaloneCoordinate      (void) const final { return false; }
    bool                specifiesCoordinate         (void) const final { return true; }
    bool                specifiesAltitudeOnly       (void) const final { return false; }
    QString             commandDescription          (void) const final { return "Takeoff Pattern"; }
    QString             commandName                 (void) const final { return "Takeoff Pattern"; }
    QString             abbreviation                (void) const final { return "T"; }
    QGeoCoordinate      coordinate                  (void) const final { return _vtolTakeoffCoordinate; }
    QGeoCoordinate      exitCoordinate              (void) const final { return _climboutCoordinate; }
    int                 sequenceNumber              (void) const final { return _sequenceNumber; }
    double              specifiedFlightSpeed        (void) final { return std::numeric_limits<double>::quiet_NaN(); }
    double              specifiedGimbalYaw          (void) final { return std::numeric_limits<double>::quiet_NaN(); }
    double              specifiedGimbalPitch        (void) final { return std::numeric_limits<double>::quiet_NaN(); }
    void                appendMissionItems          (QList<MissionItem*>& items, QObject* missionItemParent) final;
    void                applyNewAltitude            (double newAltitude) final;
    double              additionalTimeDelay         (void) const final { return 0; }
    ReadyForSaveState   readyForSaveState           (void) const final;
    bool                exitCoordinateSameAsEntry   (void) const final { return false; }
    void                setDirty                    (bool dirty) final;
    void                setCoordinate               (const QGeoCoordinate& coordinate) final { setClimboutCoordinate(coordinate); }
    void                setSequenceNumber           (int sequenceNumber) final;
    double              amslEntryAlt                (void) const final;
    double              amslExitAlt                 (void) const final;
    double              minAMSLAltitude             (void) const final { return amslEntryAlt(); }
    double              maxAMSLAltitude             (void) const final { return amslExitAlt(); }
    double              amslgroundAlt               (void);

    static const char* takeoffDistName;
    static const char* vtolAltName;
    static const char* climboutAltName;
    static const char* takeoffHeadingName;
    static const char* loiterClockwiseName;
    static const char* useLoiterToAltName;
    static const char* gradientName;
    static const char* loiterRadiusName;

signals:
    void vtolTakeoffCoordinateChanged (QGeoCoordinate coordinate);
    void climboutCoordinateChanged       (QGeoCoordinate coordinate);
    void takeoffCoordSetChanged         (bool takeoffCoordSet);
    void altitudesAreRelativeChanged    (bool altitudesAreRelative);
    void _updateFlightPathSegmentsSignal(void);

protected slots:
    virtual void _updateFlightPathSegmentsDontCallDirectly(void) = 0;

    void _recalcFromHeadingAndDistanceChange        (void);
    void _recalcFromCoordinateChange                (void);
    void _setDirty                                  (void);

protected:
    virtual const Fact*     _takeoffDist         (void) const = 0;
    virtual const Fact*     _vtolAlt               (void) const = 0;
    virtual const Fact*     _climboutAlt  (void) const = 0;
    virtual const Fact*     _takeoffHeading           (void) const = 0;
    virtual const Fact*     _loiterClockwise        (void) const = 0;
    virtual const Fact*     _useLoiterToAlt         (void) const = 0;
    virtual const Fact*     _gradient               (void) const = 0;
    virtual const Fact*     _loiterRadius           (void) const = 0;

    virtual MissionItem*    _createTakeoffItem         (int seqNum, bool altRel, double lat, double lon, double alt, QObject* parent) = 0;

    void            _init                   (void);
    QPointF         _rotatePoint            (const QPointF& point, const QPointF& origin, double angle);
    MissionItem*    _createVtolTakeoffItem  (int seqNum, QObject* parent);
    MissionItem*    _createClimboutItem     (int seqNum, QObject* parent);
    QJsonObject     _save                   (void);
    bool            _load                   (const QJsonObject& complexObject, int sequenceNumber, const QString& jsonComplexItemTypeValue, bool useDeprecatedRelAltKeys, QString& errorString);

    typedef bool                (*IsTakeoffItemFunc)(const MissionItem& missionItem);
    typedef TakeoffComplexItem* (*CreateItemFunc)(PlanMasterController* masterController, bool flyView);

    static bool _scanForItem(QmlObjectListModel* visualItems, bool flyView, PlanMasterController* masterController, IsTakeoffItemFunc IsTakeoffItemFunc, CreateItemFunc createItemFunc);

    int             _sequenceNumber             = 0;
    bool            _dirty                      = false;
    QGeoCoordinate  _vtolTakeoffCoordinate;
    QGeoCoordinate  _climboutCoordinate;
    bool            _takeoffCoordSet            = false;
    bool            _vtolTakeoffCoordSet            = false;
    bool            _ignoreRecalcSignals        = false;
    bool            _altitudesAreRelative       = true;

    static const char* _jsonVtolTakeoffCoordinateCoordinateKey;
    static const char* _jsonLoiterRadiusKey;
    static const char* _jsonLoiterClockwiseKey;
    static const char* _jsonClimboutCoordinateKey;
    static const char* _jsonUseLoiterToAltKey;


private slots:
    void    _recalcFromRadiusChange                         (void);
    void    _signalLastSequenceNumberChanged                (void);
    void    _updateClimboutCoordinateAltitudeFromFact       (void);
    void    _updateVtolTakeoffCoodinateAltitudeFromFact         (void);

    friend class TakeoffComplexItemTest;
};
