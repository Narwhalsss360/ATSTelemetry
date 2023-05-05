 #include <LiquidCrystal_I2C.h>
#include <LiteTimer.h>
#define MAIN_CONTROLLER
#include "ControllerCommons.h"
#include "Telemetry.h"
#include <Joystick.h>
#include "Display.h"

//#define BIG_DEBUG
//#define SMALL_DEBUG

//#define WHEEL_HELPER()
#define WHEEL_HELPER() Serial.println("Wheel: " + String(analogRead(pins.wheel) * 0.0977517) + '%');

#if defined(BIG_DEBUG)
#define PRINT_STATES() Serial.println("Wheel Position:" + String(analogRead(pins.wheel)) + " Accelerator Position:" + String(analogRead(pins.accelerator)) + " Brake Position:" + String(analogRead(pins.brake)))
#elif defined(SMALL_DEBUG)
#define PRINT_STATES() Serial.println("Wheel Position:" + String(analogRead(pins.wheel)))
#else
#define PRINT_STATES()
#endif

#define WHEEL_LOW_WARN 100
#define WHEEL_HIGH_WARN 920

#define MENU_BUTTON_HOLD_TIME 1000
#define SERIAL_BAUDRATE 1000000
#define JOYSTICK_ARGS JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, JOYSTICK_DEFAULT_BUTTON_COUNT, 0, false, false, false, true, true, false, false, false, true, true, true
#define MAX_TRIES 3

#define BUTTONPAD_ROWS 2
#define BUTTONPAD_COLS 5

constexpr const struct {
	const byte
		accelerator = A2,
		clutch = A3,
		brake = A1,
		wheel = A0,
		parkingBrake = 5,
		buttonPadRows[BUTTONPAD_ROWS] = { 6, 7 },
		buttonPadCols[BUTTONPAD_COLS] = { 8, 9, 10, 16, 14 };
} pins;

Push parkingBrake = Push(pins.parkingBrake, INPUT_PULLUP, DEBOUNCE);

NStreamCom serialParser = NStreamCom(&Serial);

Joystick_ joy = Joystick_(JOYSTICK_ARGS);

uint32_t lastRequest = 0;

bool buttonPadStates[10] = { false };

ONPUSH_ESR(parkingBrakePushedIn, args, {
	if (!game.truck.general.parkingBrake) {
		joy.setButton(14, true);
		joy.setButton(14, false);
	}
})

ONRELEASE_ESR(parkingBrakePulledOut, args, {
	if (game.truck.general.parkingBrake) {
		joy.setButton(14, true);
		joy.setButton(14, false);
	}
})

void serialEvent() {
	NStreamData parsedData = serialParser.parse();
	if (INVALID_DATA(parsedData)) {
		return;
	}

	switch (parsedData.id) {
	case SERIAL_IDS_NOT_USED:
		game = reinterpret_c_style(GameData, parsedData.data);
		break;
	case ID_TRUCK:
		game.truck = reinterpret_c_style(Truck, parsedData.data);
		break;
	case ID_TRAILER:
		game.trailer = reinterpret_c_style(Trailer, parsedData.data);
		break;
	case ID_COMMON:
		game.common = reinterpret_c_style(Common, parsedData.data);
		break;
	case ID_EVENTS:
		game.events = reinterpret_c_style(GameplayEventFlags, parsedData.data);
		break;
	default:
		break;
	}
	//standardMenu();
}

void wireRequest() {
	Wire.beginTransmission(WHEEL_ADDR);
	Wire.write(ID_WHEEL_DATA);
	int error = Wire.endTransmission();

	if (error) {
#if defined(SMALL_DEBUG) || defined(BIG_DEBUG)
		Serial.println("I2C Error:" + String(error) + ", Stream Error:" + Wire.getWriteError());
#endif
		return;
	};

	int n = Wire.requestFrom(WHEEL_ADDR, (int)sizeof(WheelData) + 6);
	if (n != sizeof(WheelData) + 6) {
#if defined(SMALL_DEBUG) || defined(BIG_DEBUG)
		Serial.println("Request Size Mismatch:" + String(n));
#endif
		return;
	};

	NStreamData data = wireParser.parse();

	if (INVALID_DATA(data)) {
#if defined(SMALL_DEBUG) || defined(BIG_DEBUG)
		Serial.println("Parser Error:" + String(wireParser.getLastError()));
#endif
		return;
	}

#ifdef BIG_DEBUG
	Serial.print("Received Data:");
	printArray<byte>((byte*)&wheelData, sizeof(WheelData));
#endif

	wheelData = *(WheelData*)data.data;
	joySendWheelData();
}

void joySendWheelData() {
	joy.setButton(0, bitRead(wheelData.registers[COMMON_A], CRUISE_SET));
	joy.setButton(1, bitRead(wheelData.registers[COMMON_A], CRUISE_RESUME));
	joy.setButton(2, bitRead(wheelData.registers[COMMON_A], CRUISE_CANCEL));
	joy.setButton(3, bitRead(wheelData.registers[COMMON_A], CRUISE_INCREASE));
	joy.setButton(4, bitRead(wheelData.registers[COMMON_A], CRUISE_DECREASE));
	joy.setButton(5, bitRead(wheelData.registers[COMMON_A], JOY_PUSH));
	joy.setButton(6, bitRead(wheelData.registers[COMMON_A], SIDE_SWITCH_LEFT));
	joy.setButton(7, bitRead(wheelData.registers[COMMON_A], SIDE_SWITCH_RIGHT));
	joy.setButton(8, bitRead(wheelData.registers[COMMON_B], LEFT_SWITCH_UP));
	joy.setButton(9, bitRead(wheelData.registers[COMMON_B], LEFT_SWITCH_DOWN));
	joy.setButton(10, bitRead(wheelData.registers[COMMON_B], MIDDLE_SWITCH_UP));
	joy.setButton(11, bitRead(wheelData.registers[COMMON_B], MIDDLE_SWITCH_DOWN));
	joy.setButton(12, bitRead(wheelData.registers[COMMON_B], RIGHT_SWITCH_UP));
	joy.setButton(13, bitRead(wheelData.registers[COMMON_B], RIGHT_SWITCH_DOWN));
	joy.setRxAxis(wheelData.joystick[X_AXIS]);
	joy.setRyAxis(wheelData.joystick[Y_AXIS]);
}

void joySend() {
	joy.setAccelerator(analogRead(pins.accelerator));
	joy.setRudder(analogRead(pins.clutch));
	joy.setBrake(analogRead(pins.brake));
	joy.setSteering(analogRead(pins.wheel));
	for (byte i = 0; i < 10; i++) joy.setButton(15 + i, buttonPadStates[i]);
}

void readButtonPad() {
	digitalWrite(pins.buttonPadRows[0], LOW);
	digitalWrite(pins.buttonPadRows[1], HIGH);
	for (byte i = 0; i < BUTTONPAD_COLS; i++) buttonPadStates[i] = !digitalRead(pins.buttonPadCols[i]);

	digitalWrite(pins.buttonPadRows[0], HIGH);
	digitalWrite(pins.buttonPadRows[1], LOW);
	for (byte i = 0; i < BUTTONPAD_COLS; i++) buttonPadStates[i + 5] = !digitalRead(pins.buttonPadCols[i]);
}

void setup() {
	for (byte i = 0; i < BUTTONPAD_COLS; i++) pinMode(pins.buttonPadCols[i], INPUT_PULLUP);
	for (byte i = 0; i < BUTTONPAD_ROWS; i++) pinMode(pins.buttonPadRows[i], OUTPUT);
	pinMode(4, OUTPUT);

	Wire.begin();
	Serial.begin(SERIAL_BAUDRATE);
	joy.begin();

	parkingBrake.onPush += parkingBrakePushedIn;
	parkingBrake.onRelease += parkingBrakePulledOut;
}

void loop() {
	if (Serial.available()) serialEvent();
	readButtonPad();
	wireRequest();
	joySend();
	uint16_t wheelPosition = analogRead(pins.wheel);
	Serial.println("Wheel: " + String(wheelPosition * 0.0977517106549365) + "%, Gas: " + String(map(analogRead(pins.accelerator), 0, 1023, 0, 100)) + "%, Brake: " + String(map(analogRead(pins.brake), 0, 1023, 0, 100)) + '%');

	if (wheelPosition < WHEEL_LOW_WARN || wheelPosition > WHEEL_HIGH_WARN) {
		digitalWrite(4, HIGH);
	} else {
		digitalWrite(4, LOW);
	}
}