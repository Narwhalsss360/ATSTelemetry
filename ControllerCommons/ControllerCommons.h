#ifdef MAIN_CONTROLLER
#define NSC_BUFFER_SIZE 140
#endif
#include <NStreamCom.h>
#include <Wire.h>
#include <NPush.h>

#define DEBOUNCE 15

#define TWI_SPEED 400000

#define WHEEL_ADDR 0x20
#define WHEEL_OBJECT_COUNT 2

#define DEVICE_COUNT 1

#define DEVICE_ADDRESS 0
#define OBJECTS_POINTERS 1
#define OBJECTS_SIZE_POINTER 2
#define DEVICE_OBJECT_COUNT 3

#define GET_DEVICE_ADDR(d) deviceDescriptors[d][DEVICE_ADDRESS]
#define GET_OBJECT_COUNT(d) deviceDescriptors[d][DEVICE_OBJECT_COUNT]
#define GET_OBJECT_SIZE(d, o) ((uint16_t*)deviceDescriptors[d][OBJECTS_SIZE_POINTER])[o]
#define GET_OBJECT_POINTER(d, o) (void*)((uint16_t*)(deviceDescriptors[d][OBJECTS_POINTERS]))[o]

NStreamCom wireParser = NStreamCom(&Wire);
#if defined(WHEEL_CONTROLLER) || defined(MAIN_CONTROLLER)
enum WHEEL_WIRE_IDS {
	WHEEL_WIRE_IDS_NOT_USED,
	ID_WHEEL_DATA
};

enum WHEEL_DATA_REGISTERS {
	COMMON_A,
	COMMON_B
};

enum WHEEL_COMMON_A_BITS {
	CRUISE_SET,
	CRUISE_RESUME,
	CRUISE_CANCEL,
	CRUISE_INCREASE,
	CRUISE_DECREASE,
	JOY_PUSH,
	SIDE_SWITCH_LEFT,
	SIDE_SWITCH_RIGHT
};

enum WHEEL_COMMON_B_BITS {
	LEFT_SWITCH_UP,
	LEFT_SWITCH_DOWN,
	MIDDLE_SWITCH_UP,
	MIDDLE_SWITCH_DOWN,
	RIGHT_SWITCH_UP,
	RIGHT_SWITCH_DOWN,
};

enum JOYSTICK_AXES {
	X_AXIS,
	Y_AXIS
};

struct WheelData {
	byte registers[2];
	int16_t joystick[2];
}
wheelData;
#endif

#if (defined(WHEEL_CONTROLLER) || defined(MAIN_CONTROLLER))
const constexpr uint16_t wheelControllerIndexedObjectsPointers[WHEEL_OBJECT_COUNT] = { 0x00, (uint16_t)&wheelData};
const constexpr uint16_t wheelControllerIndexedObjectsSizes[WHEEL_OBJECT_COUNT] = { 0x00, sizeof(WheelData) };
#endif

#if defined(MAIN_CONTROLLER)
enum DEVICES
{
	D_WHEEL
};

const constexpr uint16_t deviceDescriptors[DEVICE_COUNT][4] = {
	{ WHEEL_ADDR, (uint16_t)&wheelControllerIndexedObjectsPointers, (uint16_t)&wheelControllerIndexedObjectsSizes, WHEEL_OBJECT_COUNT }
};
#endif

#if !defined(MAIN_CONTROLLER)
uint8_t nextObject;
#endif

template <typename T>
void printArray(T* arr, size_t count)
{
	String out;

	for (size_t i = 0; i < count; i++)
	{
		if (i == 0) out += "[" + String(i) + "]: " + String(arr[i]);
		else out += " [" + String(i) + "]: " + String(arr[i]);
	}

	Serial.println(out);
}

void commonSetup() {
	Wire.setClock(TWI_SPEED);
}