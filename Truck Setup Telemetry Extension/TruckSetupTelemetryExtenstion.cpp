#include <Windows.h>
#include <string>
#include "scs\scssdk_telemetry.h"
#include "scs\eurotrucks2/scssdk_eut2.h"
#include "scs\eurotrucks2/scssdk_telemetry_eut2.h"
#include "scs\amtrucks/scssdk_ats.h"
#include "scs\amtrucks/scssdk_telemetry_ats.h"
#include "Telemetry.h"

#define FILE_NAME "Truck Setup Telemetry.dat"
#define MAPPED_OBJECT_NAME "TruckSetupTelemetryExtenstion"

using std::string;

string filePath;
HANDLE file = nullptr;
HANDLE map = nullptr;

bool fileExists(LPCSTR path)
{
    DWORD fileAttributes = GetFileAttributesA(path);
    return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

string getUserDirectory()
{
#pragma warning(suppress: 4996)
    return std::string(getenv("USERPROFILE"));
}

string getTempDirectory()
{   
    return getUserDirectory() + "\\AppData\\local\\temp";
}

int startMemoryShare() {
    int lastError = 0;

    if (fileExists(filePath.c_str()))
        DeleteFileA(filePath.c_str());

    file = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    lastError = GetLastError();

    if (lastError || file == NULL || file == INVALID_HANDLE_VALUE)
        return lastError;

    map = CreateFileMappingA(file, NULL, PAGE_READWRITE, 0, sizeof(GameData), MAPPED_OBJECT_NAME);
    lastError = GetLastError();

    if (lastError || map == NULL || map == INVALID_HANDLE_VALUE)
        return lastError;

    game = (GameData*)MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(GameData));

    return lastError;
}

void endMemoryShare() {
    if (file == INVALID_HANDLE_VALUE || file == nullptr)
        goto deleteLabel;

    if (game)
        UnmapViewOfFile(game);

    CloseHandle(file);

    if (map == INVALID_HANDLE_VALUE || map == nullptr)
        goto deleteLabel;

    if (map == INVALID_HANDLE_VALUE || map == nullptr)
        goto deleteLabel;

    CloseHandle(map);

deleteLabel:
    if (fileExists(filePath.c_str()))
        DeleteFileA(filePath.c_str());
}

SCSAPI_VOID storeFloat(const scs_string_t, const scs_u32_t, const scs_value_t* const value, const scs_context_t context) {
    *((float*)context) = value->value_float.value;
}

SCSAPI_VOID storeInt32(const scs_string_t, const scs_u32_t, const scs_value_t* const value, const scs_context_t context) {
    *((int32_t*)context) = value->value_s32.value;
}

SCSAPI_VOID storeUint32(const scs_string_t, const scs_u32_t, const scs_value_t* const value, const scs_context_t context) {
    *((uint32_t*)context) = value->value_u32.value;
}

SCSAPI_VOID storeBool(const scs_string_t, const scs_u32_t, const scs_value_t* const value, const scs_context_t context) {
    *((bool*)context) = value->value_bool.value;
}

SCSAPI_VOID storeShifterType(const scs_string_t, const scs_u32_t, const scs_value_t* const value, const scs_context_t context) {
    const char* const type = value->value_string.value;
    if (type == SCS_SHIFTER_TYPE_arcade) {
        (*(uint32_t*)context) = ARCADE_SHIFTER;
    } else if (type == SCS_SHIFTER_TYPE_automatic) {
        (*(uint32_t*)context) = AUTOMATIC_SHIFTER;
    } else if (type == SCS_SHIFTER_TYPE_manual) {
        (*(uint32_t*)context) = MANUAL_SHIFTER;
    } else if (type == SCS_SHIFTER_TYPE_hshifter) {
        (*(uint32_t*)context) = H_SHIFTER;
    }
}

SCSAPI_VOID gameEvent(const scs_event_t event, const void* const eventArgs, const scs_context_t context)
{
    const struct scs_telemetry_gameplay_event_t args = *(scs_telemetry_gameplay_event_t*)eventArgs;
    if (args.id == SCS_TELEMETRY_GAMEPLAY_EVENT_player_fined) {
        game->events.fined = true;

    } else if (SCS_TELEMETRY_GAMEPLAY_EVENT_player_tollgate_paid) {
        game->events.tollPayed = true;
    }
}

SCSAPI_VOID pauseEvent(const scs_event_t event, const void* const, const scs_context_t context)
{
    if (event == SCS_TELEMETRY_EVENT_paused)
    {
#ifdef _DEBUG
        ((scs_log_t)(context))(SCS_LOG_TYPE_message, "Paused");
        if (true);
#endif
        game->common.paused = true;
    }
    else
    {
        game->common.paused = false;
#ifdef _DEBUG
        ((scs_log_t)(context))(SCS_LOG_TYPE_message, "Unpaused");
#endif
    }
}

SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t* const params) {
    scs_telemetry_init_params_v101_t p = *(scs_telemetry_init_params_v101_t*)params;

    filePath = getTempDirectory() + '\\' + FILE_NAME;
    int result = startMemoryShare();

    if (result) {
        char numBuffer[100];
        sprintf_s(numBuffer, 100, "An error occured starting file-mapping. Handle error code: %d", result);
        p.common.log(SCS_LOG_TYPE_error, numBuffer);
        endMemoryShare();
        return SCS_RESULT_generic_error;
    }

#pragma region Truck Channels
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_speed, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.dashboard.speed);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_engine_rpm, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.dashboard.rpm);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_cruise_control, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.dashboard.cruiseControl);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_retarder_level, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.engine.retarder);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_brake_air_pressure, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.general.brakeAirPressure);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_brake_temperature, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.general.brakeTemperature);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_fuel, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.dashboard.fuel);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_fuel_range, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.dashboard.fuelRange);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_oil_pressure, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.engine.oilPressure);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_oil_temperature, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.engine.oilTemperature);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_battery_voltage, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.general.batteryVoltage);
    //p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_dashboard_backlight, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_odometer, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.dashboard.odometer);
    //p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_steering, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_wear_engine, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.wear.engine);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_wear_transmission, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.wear.transmission);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_wear_cabin, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.wear.cabin);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_wear_chassis, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.wear.chassis);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_wear_wheels, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->truck.wear.wheels);

    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_engine_gear, SCS_U32_NIL, SCS_VALUE_TYPE_s32, SCS_TELEMETRY_CHANNEL_FLAG_none, storeInt32, &game->truck.engine.gear);

    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_parking_brake, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->truck.general.parkingBrake);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_motor_brake, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->truck.engine.brake);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_hazard_warning, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->truck.lights.hazard);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_light_lblinker, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->truck.lights.leftBlinker);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_light_rblinker, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->truck.lights.rightBlinker);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_light_low_beam, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->truck.lights.lowBeam);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_light_high_beam, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->truck.lights.highBeam);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_wipers, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->truck.general.wipers);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_differential_lock, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->truck.engine.differentialLock);
    p.register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_engine_enabled, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->truck.engine.enabled);
#pragma endregion

#pragma region Trailer Channels
    p.register_for_channel(SCS_TELEMETRY_TRAILER_CHANNEL_wear_body, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->trailer.wear.body);
    p.register_for_channel(SCS_TELEMETRY_TRAILER_CHANNEL_wear_chassis, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->trailer.wear.chassis);
    p.register_for_channel(SCS_TELEMETRY_TRAILER_CHANNEL_wear_wheels, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->trailer.wear.wheels);
    p.register_for_channel(SCS_TELEMETRY_TRAILER_CHANNEL_cargo_damage, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->trailer.wear.cargo);

    p.register_for_channel(SCS_TELEMETRY_TRAILER_CHANNEL_connected, SCS_U32_NIL, SCS_VALUE_TYPE_bool, SCS_TELEMETRY_CHANNEL_FLAG_none, storeBool, &game->trailer.general.connected);
#pragma endregion

#pragma region Common Channels
    p.register_for_channel(SCS_TELEMETRY_CHANNEL_game_time, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, storeFloat, &game->common.gameTime);
    p.register_for_channel(SCS_TELEMETRY_CONFIG_ATTRIBUTE_shifter_type, SCS_U32_NIL, SCS_VALUE_TYPE_string, SCS_TELEMETRY_CHANNEL_FLAG_none, storeShifterType, &game->common.shifterType);
#pragma endregion

#pragma region Events
    const bool success =    (p.register_for_event(SCS_TELEMETRY_EVENT_paused, pauseEvent, p.common.log) == SCS_RESULT_ok) &&
                            (p.register_for_event(SCS_TELEMETRY_EVENT_gameplay, gameEvent, nullptr) == SCS_RESULT_ok);

    if (!success) {
        p.common.log(SCS_LOG_TYPE_error, "Could not register events!");
    }
#pragma endregion
    p.common.log(SCS_LOG_TYPE_message, "[Truck Setup Telemetry Extension] Initialized.");
    return SCS_RESULT_ok;
}

SCSAPI_VOID scs_telemetry_shutdown(void) {
    endMemoryShare();
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}