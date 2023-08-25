/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
#include "QGCApplication.h"
#include "QGCCorePlugin.h"

#include "CustomAutoPilotPlugin.h"
#include "UAS.h"
#include "APMParameterMetaData.h"
#include "APMFirmwarePlugin.h"
#include "ArduCopterFirmwarePlugin.h"
#include "ArduRoverFirmwarePlugin.h"
#include "VehicleComponent.h"
#include "APMAirframeComponent.h"
#include "APMFlightModesComponent.h"
#include "APMRadioComponent.h"
#include "APMSafetyComponent.h"
#include "APMTuningComponent.h"
#include "APMSensorsComponent.h"
#include "APMPowerComponent.h"
#include "APMMotorComponent.h"
#include "APMCameraComponent.h"
#include "APMLightsComponent.h"
#include "APMSubFrameComponent.h"
#include "APMFollowComponent.h"
#include "ESP8266Component.h"
#include "APMHeliComponent.h"
#include "QGCApplication.h"
#include "ParameterManager.h"
#include "QGCApplication.h"
#include "SettingsManager.h"

#if !defined(NO_SERIAL_LINK) && !defined(__android__)
#include <QSerialPortInfo>
#endif

/// This is the AutoPilotPlugin implementatin for the MAV_AUTOPILOT_ARDUPILOT type.
CustomAutoPilotPlugin::CustomAutoPilotPlugin(Vehicle* vehicle, QObject* parent)
    : AutoPilotPlugin           (vehicle, parent)
    , _incorrectParameterVersion(false)
    , _airframeComponent        (nullptr)
    , _cameraComponent          (nullptr)
    , _lightsComponent          (nullptr)
    , _subFrameComponent        (nullptr)
    , _flightModesComponent     (nullptr)
    , _powerComponent           (nullptr)
    , _motorComponent           (nullptr)
    , _radioComponent           (nullptr)
    , _safetyComponent          (nullptr)
    , _sensorsComponent         (nullptr)
    , _tuningComponent          (nullptr)
    , _esp8266Component         (nullptr)
    , _heliComponent            (nullptr)
#if 0
    // Follow me not ready for Stable
    , _followComponent          (nullptr)
#endif
{
#if !defined(NO_SERIAL_LINK)
    connect(vehicle->parameterManager(), &ParameterManager::parametersReadyChanged, this, &CustomAutoPilotPlugin::_checkForBadCubeBlack);
#endif
}

CustomAutoPilotPlugin::~CustomAutoPilotPlugin()
{

}

const QVariantList& CustomAutoPilotPlugin::vehicleComponents(void)
{
    bool showAdvanced = qgcApp()->toolbox()->corePlugin()->showAdvancedUI();

    if (_components.count() == 0 && !_incorrectParameterVersion) {
        if (_vehicle->parameterManager()->parametersReady()) {

            _airframeComponent = new APMAirframeComponent(_vehicle, this);
            _airframeComponent->setupTriggerSignals();
            _components.append(QVariant::fromValue((VehicleComponent*)_airframeComponent));

            _safetyComponent = new APMSafetyComponent(_vehicle, this);
            _safetyComponent->setupTriggerSignals();
            _components.append(QVariant::fromValue((VehicleComponent*)_safetyComponent));
            if (showAdvanced){
            if ( _vehicle->supportsRadio() ) {
                _radioComponent = new APMRadioComponent(_vehicle, this);
                _radioComponent->setupTriggerSignals();
                _components.append(QVariant::fromValue((VehicleComponent*)_radioComponent));
            }

            // No flight modes component for Sub versions 3.5 and up
            if (!_vehicle->sub() || (_vehicle->versionCompare(3, 5, 0) < 0)) {
                _flightModesComponent = new APMFlightModesComponent(_vehicle, this);
                _flightModesComponent->setupTriggerSignals();
                _components.append(QVariant::fromValue((VehicleComponent*)_flightModesComponent));
            }
            //broken on custom builds??? mwrightE38
          //  _sensorsComponent = new APMSensorsComponent(_vehicle, this);
         //   _sensorsComponent->setupTriggerSignals();
          //  _components.append(QVariant::fromValue((VehicleComponent*)_sensorsComponent));

            _powerComponent = new APMPowerComponent(_vehicle, this);
            _powerComponent->setupTriggerSignals();
            _components.append(QVariant::fromValue((VehicleComponent*)_powerComponent));

            _tuningComponent = new APMTuningComponent(_vehicle, this);
            _tuningComponent->setupTriggerSignals();
            _components.append(QVariant::fromValue((VehicleComponent*)_tuningComponent));

            if(_vehicle->parameterManager()->parameterExists(-1, "MNT_RC_IN_PAN")) {
                _cameraComponent = new APMCameraComponent(_vehicle, this);
                _cameraComponent->setupTriggerSignals();
                _components.append(QVariant::fromValue((VehicleComponent*)_cameraComponent));
            }
        }
        } else {
            qWarning() << "Call to vehicleCompenents prior to parametersReady";

        }
    }

    return _components;
}

QString CustomAutoPilotPlugin::prerequisiteSetup(VehicleComponent* component) const
{
    bool requiresAirframeCheck = false;

    if (qobject_cast<const APMFlightModesComponent*>(component)) {
        if (_airframeComponent && !_airframeComponent->setupComplete()) {
            return _airframeComponent->name();
        }
        if (_radioComponent && !_radioComponent->setupComplete()) {
            return _radioComponent->name();
        }
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMRadioComponent*>(component)) {
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMCameraComponent*>(component)) {
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMPowerComponent*>(component)) {
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMSafetyComponent*>(component)) {
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMTuningComponent*>(component)) {
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMSensorsComponent*>(component)) {
        requiresAirframeCheck = true;
    }

    if (requiresAirframeCheck) {
        if (_airframeComponent && !_airframeComponent->setupComplete()) {
            return _airframeComponent->name();
        }
    }

    return QString();
}

#if !defined(NO_SERIAL_LINK)
/// The following code is executed when the Vehicle is parameter ready. It checks for the service bulletin against Cube Blacks.
void CustomAutoPilotPlugin::_checkForBadCubeBlack(void)
{

    ParameterManager* paramMgr = _vehicle->parameterManager();
    AppSettings* appSettings = qgcApp()->toolbox()->settingsManager()->appSettings();

    QString swType("SYSID_SW_TYPE");
    QString batMAH("BATT_CAPACITY");
    QString trimARSPD("TRIM_ARSPD_CM");

    if (paramMgr->parameterExists(-1, swType)){

        //get sysid
        int sysid = paramMgr->getParameter(-1, swType)->rawValue().toInt();
        //E400
        if ( sysid == 10) {
            appSettings->offlineEditingVehicleName()->setRawValue(1); //map name to aircraft on connect
            appSettings->offlineEditingVehicleName()->setRawValue(10); //map name to aircraft on connect
            _vehicle->_batteryCells = 8;
            _vehicle->_batteryMAH = 16000;
            _vehicle->setDefaultRadius(70,500);
            qDebug() << "E400";
        }
        //E455
        if (sysid == 15) {
            appSettings->offlineEditingVehicleName()->setRawValue(1); //map name to aircraft on connect
            appSettings->offlineEditingVehicleName()->setRawValue(15); //map name to aircraft on connect
            _vehicle->_batteryCells = 12;
            _vehicle->_batteryMAH = 30000;
            _vehicle->setDefaultRadius(120,500);
            qDebug() << "E455";
        }
        //check if battery capacity available
        if (paramMgr->parameterExists(-1, batMAH)){
           _vehicle->_batteryMAH  = paramMgr->getParameter(-1, batMAH)->rawValue().toInt();
        }
        //check if airspeed trim available
        if (paramMgr->parameterExists(-1, trimARSPD)){
        int trimAirspeed = paramMgr->getParameter(-1, trimARSPD)->rawValue().toInt()/100;
        appSettings->offlineEditingCruiseSpeed()->setRawValue((double)trimAirspeed);
        emit appSettings->offlineEditingCruiseSpeed()->rawValueChanged((double)trimAirspeed);
        }

    }

    bool cubeBlackFound = false;
#if 0
    // FIXME: Put back
    for (const QVariant& varLink: _vehicle->links()) {
        SerialLink* serialLink = varLink.value<SerialLink*>();
        if (serialLink && QSerialPortInfo(*serialLink->_hackAccessToPort()).description().contains(QStringLiteral("CubeBlack"))) {
            cubeBlackFound = true;
        }

    }
#endif
    if (!cubeBlackFound) {
        return;
    }

    QString paramAcc3("INS_ACC3_ID");
    QString paramGyr3("INS_GYR3_ID");
    QString paramEnableMask("INS_ENABLE_MASK");

    if (paramMgr->parameterExists(-1, paramAcc3) && paramMgr->getParameter(-1, paramAcc3)->rawValue().toInt() == 0 &&
            paramMgr->parameterExists(-1, paramGyr3) && paramMgr->getParameter(-1, paramGyr3)->rawValue().toInt() == 0 &&
            paramMgr->parameterExists(-1, paramEnableMask) && paramMgr->getParameter(-1, paramEnableMask)->rawValue().toInt() >= 7) {
        qgcApp()->showAppMessage(tr("WARNING: The flight board you are using has a critical service bulletin against it which advises against flying. For details see: https://discuss.cubepilot.org/t/sb-0000002-critical-service-bulletin-for-cubes-purchased-between-january-2019-to-present-do-not-fly/406"));

    }
}
#endif
