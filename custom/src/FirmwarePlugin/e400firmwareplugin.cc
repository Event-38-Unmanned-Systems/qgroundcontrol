/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "e400firmwareplugin.h"
#include "CustomAutoPilotPlugin.h"
#include "FirmwarePlugin.h"

bool E400FirmwarePlugin::_remapParamNameIntialized = false;
FirmwarePlugin::remapParamNameMajorVersionMap_t E400FirmwarePlugin::_remapParamName;

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

E400FirmwarePlugin::E400FirmwarePlugin(void)
{
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

int E400FirmwarePlugin::remapParamNameHigestMinorVersionNumber(int majorVersionNumber) const
{
    // Remapping supports up to 3.10
    return majorVersionNumber == 3 ? 10 : Vehicle::versionNotSetValue;
}

