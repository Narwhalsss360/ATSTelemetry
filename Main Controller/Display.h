#define LCD_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4
#define LCD_FRAMERATE 10hz

#define LEFT_BLINKER_CHAR (char)0x7F
#define RIGHT_BLINKER_CHAR (char)0x7E

#define FUEL_WARNING_A_CHARACTER 0
#define FUEL_WARNING_B_CHARACTER 1

#define CHECK_ENGINE_A_CHARACTER 2
#define CHECK_ENGINE_B_CHARACTER 3
#define APPEND_CHECK_ENGINE (('E') + String((char)CHECK_ENGINE_A_CHARACTER) + String((char)CHECK_ENGINE_B_CHARACTER))
#define COND_CHECK_ENGINE(cond) ((cond) ? (' ') + String((char)CHECK_ENGINE_A_CHARACTER) + String((char)CHECK_ENGINE_B_CHARACTER) : F("   "))

#define HEADLIGHT_CHARACTER 4
#define APPEND_HEADLIGHT (char)HEADLIGHT_CHARACTER

#define HIGH_BEAM_CHARACTER 5
#define APPEND_HIGH_BEAM (String((char)HEADLIGHT_CHARACTER) + String((char)HIGH_BEAM_CHARACTER))
#define COND_HIGH_BEAM(cond) ((cond) ? String((char)HEADLIGHT_CHARACTER) + String((char)HIGH_BEAM_CHARACTER) : HEADLIGHT_CHARACTER + (' '))

#define APPEND_LOW_BEAM (String((char)HEADLIGHT_CHARACTER) + String((char)LOW_BEAM_CHARACTER))
#define LOW_BEAM_CHARACTER 6
#define COND_LOW_BEAM(cond) ((cond) ? String((char)HEADLIGHT_CHARACTER) + String((char)LOW_BEAM_CHARACTER): HEADLIGHT_CHARACTER + (' '))

#define HIGH_OIL_TEMPERATURE_CHARACTER 7
#define COND_HIGH_OIL_TEMPERATURE(cond) ((cond) ? "O" + String((char)HIGH_OIL_TEMPERATURE_CHARACTER) : "  ")

uint8_t fuelWarningCharacterA[8] = { 0x1F, 0x11, 0x11, 0x1F, 0x0E, 0x0F, 0x0E, 0x1F };
uint8_t fuelWarningCharacterB[8] = { 0x00, 0x12, 0x1A, 0x0A, 0x0A, 0x10, 0x02, 0x00 };

uint8_t checkEngineCharacterA[8] = { 0x01, 0x06, 0x16, 0x1E, 0x1E, 0x16, 0x03, 0x01 };
uint8_t checkEngineCharacterB[8] = { 0x18, 0x05, 0x1D, 0x07, 0x1F, 0x05, 0x1D, 0x10 };

uint8_t headlightCharacter[8] = { 0x01, 0x07, 0x1F, 0x1F, 0x1F, 0x1F, 0x07, 0x01 };
uint8_t highBeamCharacter[8] = { 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x1F };
uint8_t lowBeamCharacter[8] = { 0x1C, 0x00, 0x1C, 0x00, 0x00, 0x1C, 0x00, 0x1C };

uint8_t highOilTemperatureCharacter[8] = { 0x0E, 0x08, 0x0E, 0x08, 0x0E, 0x08, 0x0C, 0x0C };

enum LCD_MENU
{
	STANDARD_MENU,
	NOTIFICATION_MENU
} static menu;

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(LCD_ADDR, LCD_COLS, LCD_ROWS);
LiteTimer lcdFrame = LiteTimer(LCD_FRAMERATE, true);

static void registerCustomCharacters() {
	lcd.createChar(FUEL_WARNING_A_CHARACTER, fuelWarningCharacterA);
	lcd.createChar(FUEL_WARNING_B_CHARACTER, fuelWarningCharacterB);

	lcd.createChar(CHECK_ENGINE_A_CHARACTER, checkEngineCharacterA);
	lcd.createChar(CHECK_ENGINE_B_CHARACTER, checkEngineCharacterB);

	lcd.createChar(HEADLIGHT_CHARACTER, headlightCharacter);
	lcd.createChar(LOW_BEAM_CHARACTER, lowBeamCharacter);
	lcd.createChar(HIGH_BEAM_CHARACTER, highBeamCharacter);

	lcd.createChar(HIGH_OIL_TEMPERATURE_CHARACTER, highOilTemperatureCharacter);
}

void lcdSetup() {
	lcd.init();
	lcd.backlight();
	registerCustomCharacters();
}

inline void cycleNextMenu() {
	return; //Does nothing now with one menu
	if (menu == NOTIFICATION_MENU) menu = STANDARD_MENU;
	else menu = (LCD_MENU)(menu + 1);
}

inline void setMenu(LCD_MENU requestedMenu) {
	if (menu != NOTIFICATION_MENU) menu = requestedMenu;
}

inline LCD_MENU getCurrentMenu() {
	return menu;
}

inline void notify(String str) {
	menu = NOTIFICATION_MENU;
}

void standardMenu()
{
}

void standardMenuOld() {
	String lines[2];

#pragma region Line 1
	lines[0] += (game.truck.lights.leftBlinker) ? LEFT_BLINKER_CHAR : '-';
	lines[0] += ' ';

	if (game.truck.lights.highBeam) {
		lines[0] += APPEND_HIGH_BEAM;
	} else if (game.truck.lights.lowBeam) {
		lines[0] += APPEND_LOW_BEAM;
	} else {
		lines[0] += APPEND_HEADLIGHT;
		lines[0] += ' ';
	}
	lines[0] += ' ';

	lines[0] += (game.truck.general.parkingBrake) ? F("(P)") : F("   ");
	lines[0] += ' ';

	if (FLASH_ENGINE_LIGHT) {
		lines[0] += APPEND_CHECK_ENGINE;
	} else if (FLASH_WARNING_LIGHT) {

	} else {
		lines[0] += "   ";
	}
	lines[0] += ' ';

	lines[0] += COND_HIGH_OIL_TEMPERATURE(FLASH_HIGH_OIL_TEMPERATURE);
	lines[0] += ' ';

	if (FLASH_LOW_FUEL_WARNING) {
		lines[0] += (char)FUEL_WARNING_A_CHARACTER; //Since 0, cannot use complex concat
		lines[0] += (char)FUEL_WARNING_B_CHARACTER;
	} else {
		lines[0] += "  ";
	}

	lines[0] += ' ';

	lines[0] += (game.truck.lights.rightBlinker) ? RIGHT_BLINKER_CHAR : '-';
#pragma endregion

#pragma region Line 2
	lines[1];

	game.truck.dashboard.speed *= 2.23694;

	if (game.truck.dashboard.speed < 10) lines[1] += ' ';
	lines[1] += (int)game.truck.dashboard.speed;
	lines[1] += "mph";

	lines[1] += (game.truck.engine.brake) ? '*' : ' ';

	if (game.truck.dashboard.rpm < 10) {
		lines[1] += "   ";
	} else if (game.truck.dashboard.rpm < 100) {
		lines[1] += "  ";
	} else if (game.truck.dashboard.rpm < 1000) {
		lines[1] += ' ';
	}

	lines[1] += (int)game.truck.dashboard.rpm;
	lines[1] += "rpm ";

	if (game.truck.engine.gear < 0) {
		lines[1] += 'R';
	} else {
		if (game.common.shifterType == AUTOMATIC_SHIFTER || ARCADE_SHIFTER) {
			lines[1] += 'A';
		} else {
			lines[1] += 'M';
		}
	}

	lines[1] += abs(game.truck.engine.gear);
	if (game.truck.engine.gear < 10) {
		lines[1] += ' ';
	}

	lines[1] += ' ';

	game.truck.dashboard.cruiseControl *= 2.23694;

	if (game.truck.dashboard.cruiseControl < 10) {
		lines[1] += ' ';
	}

	lines[1] += (int)game.truck.dashboard.cruiseControl;

#pragma endregion
	for (size_t line = 0; line < 2; line++)
	{
		lcd.setCursor(0, line);
		const char* lineString = lines[line].c_str();
		for (size_t column = 0; column < 20; column++) lcd.write(lineString[column]);
	}
}

void dismissNotification() {
	menu = STANDARD_MENU;
}

void displayFrame() {
	switch (menu) {
	case STANDARD_MENU: standardMenu(); break;
	default: break;
	}
}