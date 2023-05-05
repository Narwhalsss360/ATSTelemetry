//Pin 13 internal pullup resistor is bad,
//analog reading A3 reads 0 every other read

#define TX_LED 0
#define RX_LED 1

#define FLASH_PIN(p) digitalWrite(p, HIGH); digitalWrite(p, LOW)

#define WHEEL_CONTROLLER
#include "ControllerCommons.h"

#define CRUISE_DEBOUNCE 0
#define CRUISE_LONG_HOLD 1000
#define SWITCH_LONG_HOLD

#define RESET_CRUISE_BUTTONS() wheelData.registers[COMMON_A] = 0
#define SWITCH_DOWN_LOWER_BOUNDARY 650
#define SWITCH_DOWN_UPPER_BOUNDARY 900
#define SWITCH_UP_LOWER_BOUNDARY 1000

#define READ_DOWN(pin) (SWITCH_DOWN_LOWER_BOUNDARY < analogRead(pin) && analogRead(pin) < SWITCH_DOWN_UPPER_BOUNDARY)
#define READ_UP(pin) (analogRead(pin) > SWITCH_UP_LOWER_BOUNDARY)

constexpr const struct {
	const byte
		cruiseMain = 5,
		cruiseA = 3,
		cruiseB = 4,
		sideSwitch = A3,
		leftSwitch = A2,
		middleSwitch = A0,
		rightSwitch = A1,
		joyx = A6,
		joyy = A7,
		joyb = 13;
} pins;

VirtualPush cruiseUp = VirtualPush([]() { return digitalRead(pins.cruiseA) && !digitalRead(pins.cruiseB); }, CRUISE_DEBOUNCE);
VirtualPush cruiseDown = VirtualPush([]() { return !digitalRead(pins.cruiseA) && digitalRead(pins.cruiseB); }, CRUISE_DEBOUNCE);

bool lastMain = false;
bool checkCancel() {
	bool current = digitalRead(pins.cruiseMain);
	if (current != lastMain) {
		lastMain = current;
		if (current == HIGH) {
			return true;
		};
	}

	return digitalRead(pins.cruiseA) && digitalRead(pins.cruiseB);
}
VirtualPush cruiseCancel = VirtualPush(checkCancel, CRUISE_DEBOUNCE);

VirtualPush sideSwitchLeft = VirtualPush([]() { return READ_UP(pins.sideSwitch);; }, DEBOUNCE);
VirtualPush sideSwitchRight = VirtualPush([]() { return READ_DOWN(pins.sideSwitch); }, DEBOUNCE);

VirtualPush leftSwitchUp = VirtualPush([]() { return READ_UP(pins.leftSwitch); }, DEBOUNCE);
VirtualPush leftSwitchDown = VirtualPush([]() { return READ_DOWN(pins.leftSwitch); }, DEBOUNCE);

VirtualPush middleSwitchUp = VirtualPush([]() { return READ_UP(pins.middleSwitch);; }, DEBOUNCE);
VirtualPush middleSwitchDown = VirtualPush([]() { return READ_DOWN(pins.middleSwitch); }, DEBOUNCE);

VirtualPush rightSwitchUp = VirtualPush([]() { return READ_UP(pins.rightSwitch);; }, DEBOUNCE);
VirtualPush rightSwitchDown = VirtualPush([]() { return READ_DOWN(pins.rightSwitch); }, DEBOUNCE);

void onReceive(int) {
	if (!Serial) FLASH_PIN(RX_LED);
}

void onRequest() {
	if (!Serial) FLASH_PIN(TX_LED);
	updateVirtualButtons();
	storeInputs();
	wireParser.send(1, &wheelData, sizeof(WheelData));
	wheelData.registers[0] &= 0;
}

ONRELEASE_ESR(cruiseUpRelease, releaseData, {
	if (digitalRead(pins.cruiseMain)) return;

	if (releaseData.holdTime > CRUISE_LONG_HOLD) {
		bitSet(wheelData.registers[COMMON_A], CRUISE_RESUME);
	} else {
		bitSet(wheelData.registers[COMMON_A], CRUISE_INCREASE);
	}
})

ONRELEASE_ESR(cruiseDownRelease, releaseData, {
	if (digitalRead(pins.cruiseMain)) return;

	if (releaseData.holdTime > CRUISE_LONG_HOLD) {
		bitSet(wheelData.registers[COMMON_A], CRUISE_SET);
	} else {
		bitSet(wheelData.registers[COMMON_A], CRUISE_DECREASE);
	}
})

void updateVirtualButtons() {
	cruiseUp.update();
	cruiseDown.update();
}

void storeInputs() {
	wheelData.joystick[X_AXIS] = analogRead(pins.joyx);
	wheelData.joystick[Y_AXIS] = analogRead(pins.joyy);

	bitWrite(wheelData.registers[COMMON_A], CRUISE_CANCEL, (cruiseCancel.current()));

	bitWrite(wheelData.registers[COMMON_A], JOY_PUSH, !digitalRead(pins.joyb));
	analogRead(pins.sideSwitch); //Do this first, arduino is broken or something but every 2nd read is 0, for some reason.

	bitWrite(wheelData.registers[COMMON_A], SIDE_SWITCH_LEFT, sideSwitchLeft.current());
	bitWrite(wheelData.registers[COMMON_A], SIDE_SWITCH_RIGHT, sideSwitchRight.current());

	bitWrite(wheelData.registers[COMMON_B], LEFT_SWITCH_UP, leftSwitchUp.current());
	bitWrite(wheelData.registers[COMMON_B], LEFT_SWITCH_DOWN, leftSwitchDown.current());

	bitWrite(wheelData.registers[COMMON_B], MIDDLE_SWITCH_UP, middleSwitchUp.current());
	bitWrite(wheelData.registers[COMMON_B], MIDDLE_SWITCH_DOWN, middleSwitchDown.current());

	bitWrite(wheelData.registers[COMMON_B], RIGHT_SWITCH_UP, rightSwitchUp.current());
	bitWrite(wheelData.registers[COMMON_B], RIGHT_SWITCH_DOWN, rightSwitchDown.current());
}

void setup() {
	cruiseUp.onRelease += cruiseUpRelease;
	cruiseDown.onRelease += cruiseDownRelease;

	pinMode(pins.sideSwitch, INPUT);
	pinMode(pins.joyb, INPUT_PULLUP);

	Wire.begin(WHEEL_ADDR);
	Wire.onRequest(onRequest);
	Wire.onReceive(onReceive);

	pinMode(pins.cruiseMain, INPUT_PULLUP);
	pinMode(pins.cruiseA, INPUT);
	pinMode(pins.cruiseB, INPUT);

	if (!Serial) {
		pinMode(TX_LED, OUTPUT);
		pinMode(RX_LED, OUTPUT);
	}
}

void loop() {
}