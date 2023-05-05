#define ROT_DEBOUCE 25
#define ROT_PUSH_DEBOUNCE 30

#define NSC_BUFFER_SIZE 130

#include "Telemetry.h"
#include <NStreamCom.h>
#include <NRotary.h>
#include <NPush.h>
#include <NStreamCom.h>
#include "Pins.h"
#include "Displays.h"

struct
{
	Rotary rotary = Rotary(pins.rotary[P_ROTA], pins.rotary[P_ROTB], INPUT_PULLUP, true, ROT_DEBOUCE);
	Push push = Push(pins.rotary[P_ROTP], INPUT_PULLUP, ROT_PUSH_DEBOUNCE);
} rotary;

NStreamCom serialParser = NStreamCom(&Serial);

void serialEvent()
{
	NStreamData parsedData = serialParser.parse();

	if (INVALID_DATA(parsedData))
	{
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		return;
	};

	switch (parsedData.id)
	{
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

	game.truck.dashboard.speed *= 2.23693629;
	game.truck.dashboard.cruiseControl *= 2.23693629;
}

ONPUSH_ESR(rotaryPushed, args,
{

})

ONRELEASE_ESR(rotaryReleased, args,
{

})

void rotaryEvent()
{
	rotary.rotary.serviceRoutine();
}

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	rotary.push.onPush += rotaryPushed;
	rotary.push.onRelease += rotaryReleased;
	attachInterrupt(digitalPinToInterrupt(pins.rotary[P_ROTA]), rotaryEvent, rotary.rotary.mode);

	SETUP_DISPLAYS();

	Serial.begin(1000000);
	displays[TOP].showString("------");
	displays[BOTTOM].showString("SER");
	//while (!Serial);
}

void loop()
{
	if (rotary.push.current())
	{
		switch (rotary.rotary.getState())
		{
		case IDLE: break;
		case COUNTER_CLOCKWISE: if (view > 0) view--; break;
		case CLOCKWISE: if (view < VIEWS_MAX) view++; break;
		default: break;
		}
	}
	else
	{
		switch (rotary.rotary.getState())
		{
		case IDLE:
			break;
		case COUNTER_CLOCKWISE:
			if (displayBrightness > DISPLAY_MIN_BRIGHTNESS) displayBrightness--;
			DUAL_DISP_EXPR(.setBrightness(displayBrightness));
			break;
		case CLOCKWISE:
			if (displayBrightness < DISPLAY_MAX_BRIGHTNESS) displayBrightness++;
			DUAL_DISP_EXPR(.setBrightness(displayBrightness));
			break;
		default:
			break;
		}
	}

	runDisplays();
}