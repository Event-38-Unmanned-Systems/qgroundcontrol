
#include "CustomFirmwarePlugin.h"
#include "CustomAutoPilotPlugin.h"
#include "APMFlightModesComponentController.h"
#include "APMAirframeComponentController.h"
#include "APMSensorsComponentController.h"

#include "APMFlightModesComponentController.h"
#include "APMAirframeComponentController.h"
#include "APMSensorsComponentController.h"
#include "APMFollowComponentController.h"
#include "APMSubMotorComponentController.h"


CustomFirmwarePlugin::CustomFirmwarePlugin()
{
    qmlRegisterType<APMFlightModesComponentController>  ("QGroundControl.Controllers", 1, 0, "APMFlightModesComponentController");
    qmlRegisterType<APMAirframeComponentController>     ("QGroundControl.Controllers", 1, 0, "APMAirframeComponentController");
    qmlRegisterType<APMSensorsComponentController>      ("QGroundControl.Controllers", 1, 0, "APMSensorsComponentController");
    qmlRegisterType<APMFollowComponentController>       ("QGroundControl.Controllers", 1, 0, "APMFollowComponentController");
    qmlRegisterType<APMSubMotorComponentController>     ("QGroundControl.Controllers", 1, 0, "APMSubMotorComponentController");

    setSupportedModes({
        E400PlaneMode(E400PlaneMode::MANUAL,          false),
        E400PlaneMode(E400PlaneMode::CIRCLE,          false),
        E400PlaneMode(E400PlaneMode::STABILIZE,       false),
        E400PlaneMode(E400PlaneMode::TRAINING,        false),
        E400PlaneMode(E400PlaneMode::ACRO,            false),
        E400PlaneMode(E400PlaneMode::FLY_BY_WIRE_A,   false),
        E400PlaneMode(E400PlaneMode::FLY_BY_WIRE_B,   false),
        E400PlaneMode(E400PlaneMode::CRUISE,          false),
        E400PlaneMode(E400PlaneMode::AUTOTUNE,        false),
        E400PlaneMode(E400PlaneMode::AUTO,            true),
        E400PlaneMode(E400PlaneMode::RTL,             true),
        E400PlaneMode(E400PlaneMode::LOITER,          true),
        E400PlaneMode(E400PlaneMode::TAKEOFF,         false),
        E400PlaneMode(E400PlaneMode::AVOID_ADSB,      false),
        E400PlaneMode(E400PlaneMode::GUIDED,          false),
        E400PlaneMode(E400PlaneMode::INITIALIZING,    false),
        E400PlaneMode(E400PlaneMode::QSTABILIZE,      false),
        E400PlaneMode(E400PlaneMode::QHOVER,          false),
        E400PlaneMode(E400PlaneMode::QLOITER,         false),
        E400PlaneMode(E400PlaneMode::QLAND,           true),
        E400PlaneMode(E400PlaneMode::QRTL,            false),
        E400PlaneMode(E400PlaneMode::QAUTOTUNE,       false),
        E400PlaneMode(E400PlaneMode::QACRO,           false),
        E400PlaneMode(E400PlaneMode::THERMAL,         false),
    });

    if (!_remapParamNameIntialized) {
        FirmwarePlugin::remapParamNameMap_t& remapV3_10 = _remapParamName[3][10];

        remapV3_10["BATT_ARM_VOLT"] =    QStringLiteral("ARMING_VOLT_MIN");
        remapV3_10["BATT2_ARM_VOLT"] =   QStringLiteral("ARMING_VOLT2_MIN");

        _remapParamNameIntialized = true;
    }
}

//-----------------------------------------------------------------------------
AutoPilotPlugin* CustomFirmwarePlugin::autopilotPlugin(Vehicle* vehicle)
{
    return new CustomAutoPilotPlugin(vehicle, vehicle);
}

const QVariantList& CustomFirmwarePlugin::toolIndicators(const Vehicle* vehicle)
{
    if (_toolIndicatorList.size() == 0) {
        // First call the base class to get the standard QGC list. This way we are guaranteed to always get
        // any new toolbar indicators which are added upstream in our custom build.
        _toolIndicatorList = FirmwarePlugin::toolIndicators(vehicle);
        // Then specifically remove the RC RSSI indicator.
        _toolIndicatorList.removeOne(QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/RCRSSIIndicator.qml")));
    }
    return _toolIndicatorList;
}

bool CustomFirmwarePlugin::_remapParamNameIntialized = false;
FirmwarePlugin::remapParamNameMajorVersionMap_t CustomFirmwarePlugin::_remapParamName;

E400PlaneMode::E400PlaneMode(uint32_t mode, bool settable)
    : APMCustomMode(mode, settable)
{
    setEnumToStringMapping({
        { MANUAL,           "Manual" },
        { CIRCLE,           "Circle" },
        { STABILIZE,        "Stabilize" },
        { TRAINING,         "Training" },
        { ACRO,             "Acro" },
        { FLY_BY_WIRE_A,    "FBW A" },
        { FLY_BY_WIRE_B,    "FBW B" },
        { CRUISE,           "Cruise" },
        { AUTOTUNE,         "Autotune" },
        { AUTO,             "Auto" },
        { RTL,              "RTL" },
        { LOITER,           "Loiter" },
        { TAKEOFF,          "Takeoff" },
        { AVOID_ADSB,       "Avoid ADSB" },
        { GUIDED,           "Guided" },
        { INITIALIZING,     "Initializing" },
        { QSTABILIZE,       "QuadPlane Stabilize" },
        { QHOVER,           "QuadPlane Hover" },
        { QLOITER,          "QuadPlane Loiter" },
        { QLAND,            "QuadPlane Land" },
        { QRTL,             "QuadPlane RTL" },
        { QAUTOTUNE,        "QuadPlane AutoTune" },
        { QACRO,            "QuadPlane Acro" },
        { THERMAL,          "Thermal"},
    });
}

int CustomFirmwarePlugin::remapParamNameHigestMinorVersionNumber(int majorVersionNumber) const
{
    // Remapping supports up to 3.10
    return majorVersionNumber == 3 ? 10 : Vehicle::versionNotSetValue;
}

QList<MAV_CMD> CustomFirmwarePlugin::supportedMissionCommands(QGCMAVLink::VehicleClass_t vehicleClass)
{
    QList<MAV_CMD> supportedCommands = {
        MAV_CMD_NAV_WAYPOINT,
        MAV_CMD_NAV_LOITER_TURNS, MAV_CMD_NAV_LOITER_TIME,
        MAV_CMD_NAV_LOITER_TO_ALT,
        MAV_CMD_DO_JUMP,
        MAV_CMD_DO_SET_HOME,
        MAV_CMD_DO_SET_RELAY, MAV_CMD_DO_REPEAT_RELAY,
        MAV_CMD_DO_SET_SERVO, MAV_CMD_DO_REPEAT_SERVO,
        MAV_CMD_DO_LAND_START,
        MAV_CMD_DO_SET_ROI,
        MAV_CMD_DO_SET_CAM_TRIGG_DIST,
        MAV_CMD_NAV_VTOL_TAKEOFF, MAV_CMD_NAV_VTOL_LAND,
    };
    return supportedCommands;
}

void CustomFirmwarePlugin::guidedModeLand(Vehicle* vehicle)
{
    vehicle->sendMavCommand(
           vehicle->defaultComponentId(),                                                                    // Target component
           MAV_CMD_DO_LAND_START,                                                // Command id
           0,                                                                      // ShowError
           0,                                                                          // Reserved (Set to 0)
           0,   // Duration between two consecutive pictures (in seconds--ignored if single image)
           0,
           0,
           0,
           0);

}
