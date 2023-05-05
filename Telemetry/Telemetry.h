#include <inttypes.h>

#define USING_METRIC //Celcius, Kilometers

#define ENGINE_LIGHT_THRESHOLD 0.25
#define FLASH_ENGINE_LIGHT (game.truck.wear.engine > ENGINE_LIGHT_THRESHOLD)

#define FLASH_WARNING_LIGHT (false)

#define HIGH_OIL_TEMPERATURE_THRESHOLD 125
#define FLASH_HIGH_OIL_TEMPERATURE (game.truck.engine.oilTemperature > HIGH_OIL_TEMPERATURE_THRESHOLD)

#define LOW_FUEL_RANGE_THRESHOLD 80
#define FLASH_LOW_FUEL_WARNING (game.truck.dashboard.fuelRange < LOW_FUEL_RANGE_THRESHOLD)

#ifdef WIN32
#define GAMEVARMOD *
#else
#define GAMEVARMOD
#endif

enum SHIFTER_TYPES
{
	ARCADE_SHIFTER,
	AUTOMATIC_SHIFTER,
	MANUAL_SHIFTER,
	H_SHIFTER
};

#pragma pack(1)
struct Truck {
	struct {
		float
			speed,
			rpm,
			fuel,
			fuelRange,
			odometer,
			cruiseControl;
	} dashboard; //24


#pragma pack(1)
	struct {
		float
			retarder,
			oilPressure,
			oilTemperature;
		
		int32_t
			gear;

		bool
			enabled,
			brake,
			differentialLock;
	} engine; //20 for some reason not 19

#pragma pack(1)
	struct {
		bool
			hazard,
			lowBeam,
			highBeam,
			leftBlinker,
			rightBlinker;
	} lights; //5

#pragma pack(1)
	struct {
		float
			batteryVoltage,
			brakeAirPressure,
			brakeTemperature;
		bool
			wipers,
			parkingBrake;
	} general; //16 not 15

#pragma pack(1)
	struct {
		float
			engine,
			transmission,
			cabin,
			chassis,
			wheels;
	} wear; //20
}; //80 Bytes

#pragma pack(1)
struct Trailer {
#pragma pack(1)
	struct {
		bool
			connected;
	} general;

#pragma pack(1)
	struct {
		float
			body,
			chassis,
			wheels,
			cargo;
	} wear;
};

#pragma pack(1)
struct Common {
	bool
		paused;

	uint32_t
		gameTime,
		shifterType;
};

#pragma pack(1)
struct GameplayEventFlags {
	bool
		fined,
		tollPayed;
};

#pragma pack(1)
struct GameData {
	Truck truck;
	Trailer trailer;
	Common common;
	GameplayEventFlags events;
} GAMEVARMOD game;

enum SERIAL_IDS {
	SERIAL_IDS_NOT_USED,
	ID_TRUCK,
	ID_TRAILER,
	ID_COMMON,
	ID_EVENTS
};