#include "Pins.h"
#include <TM1637TinyDisplay6.h>

#define DUAL_DISP_EXPR(expr) displays[0]expr; displays[1]expr
#define DISP_EXPR(top_expr, bottom_expr) displays[0]top_expr; displays[1]bottom_expr
#define CLEAR_DISPLAYS() DUAL_DISP_EXPR(.showString("      "))
#define SETUP_DISPLAYS() DUAL_DISP_EXPR(.setBrightness(255));
#define DISPLAY_MIN_BRIGHTNESS 0
#define DISPLAY_MAX_BRIGHTNESS 7
#define VIEWS_MAX 2

enum DISPLAYS
{
	TOP,
	BOTTOM
};

uint8_t view = 0;
byte displayBrightness = DISPLAY_MAX_BRIGHTNESS;
TM1637TinyDisplay6 displays[2] = { TM1637TinyDisplay6(pins.displays[TOP][P_CLK], pins.displays[TOP][P_DIO]), TM1637TinyDisplay6(pins.displays[BOTTOM][P_CLK], pins.displays[BOTTOM][P_DIO]) };

void standardDisplayTop()
{
	String line;
    
    line += (int)game.truck.dashboard.cruiseControl;
    if (game.truck.dashboard.cruiseControl < 10) line += ' ';
    if (game.truck.dashboard.cruiseControl < 100) line += ' ';

    if (game.truck.dashboard.speed < 10) line += ' ';
    if (game.truck.dashboard.speed < 100) line += ' ';
    line += abs((int)game.truck.dashboard.speed);

    displays[TOP].showString(line.c_str());
}

void standardDisplayBottom()
{
    String line;
    line += (int)game.truck.dashboard.rpm;
    if (game.truck.dashboard.rpm < 1000) line += ' ';
    if (game.truck.dashboard.rpm < 100) line += ' ';
    if (game.truck.dashboard.rpm < 10) line += ' ';
    line += " r";

    displays[BOTTOM].showString(line.c_str());
}

void standardDisplay()
{
    standardDisplayTop();
    standardDisplayBottom();
}

const char gearBoxMarker()
{
    if (game.truck.engine.gear < 0) return 'R';
    switch (game.common.shifterType)
    {
    case ARCADE_SHIFTER:
    case AUTOMATIC_SHIFTER: return 'A';
    case MANUAL_SHIFTER: return 'S';
    case H_SHIFTER: return (game.truck.engine.gear % 2 == 0) ? 'H' : 'L';
    default: break;
    }
}

void secondaryDisplayTop()
{
    String line;
    line += gearBoxMarker() ;
    if (game.truck.engine.gear < 0)
    {
        line += abs(game.truck.engine.gear);
        line += '-';
    }
    else if (game.common.shifterType == H_SHIFTER)
    {
        line += game.truck.engine.gear / 2;
        line += '-';
    }
    else
    {
        line += game.truck.engine.gear;
        if (game.truck.engine.gear < 10) line += '-';
    }

    line += ' ';
    if (game.truck.dashboard.speed < 10) line += ' ';
    line += abs(int(game.truck.dashboard.speed));

    displays[TOP].showString(line.c_str());
}

void secondaryDisplayBottom()
{
    String line;
    line += int(game.truck.dashboard.fuelRange / 1.609);
    line += "rA";
    displays[BOTTOM].showString(line.c_str());
}

void secondaryDisplay()
{
    secondaryDisplayTop();
    secondaryDisplayBottom();
}

void tertiaryDisplayTop()
{
    String line;
    line += (game.truck.lights.leftBlinker) ? '-' : ' ';

    line += int(game.truck.dashboard.cruiseControl);
    if (game.truck.dashboard.cruiseControl < 10) line += ' ';

    line += abs(game.truck.engine.gear);

    if (game.truck.engine.gear < 10) line += ' ';

    line += (game.truck.lights.rightBlinker) ? '-' : ' ';
    
    displays[TOP].showString(line.c_str());
}

void tertiaryDisplayBottom()
{
    String line;
    line += abs(int(game.truck.dashboard.speed));
    if (game.truck.dashboard.speed < 10) line += ' ';

    if (game.truck.dashboard.rpm < 1000) line += ' ';
    if (game.truck.dashboard.rpm < 100) line += ' ';
    if (game.truck.dashboard.rpm < 10) line += ' ';
    
    line += int(game.truck.dashboard.rpm);

    displays[BOTTOM].showString(line.c_str());
}

void tertiaryDisplay()
{
    tertiaryDisplayTop();
    tertiaryDisplayBottom();
}

void runDisplays()
{
    if (game.truck.engine.enabled)
    {
        switch (view)
        {
        case 0:
            standardDisplay();
            break;
        case 1:
            secondaryDisplay();
            break;
        case 2:
            tertiaryDisplay();
            break;
        default:
            break;
        }
    }
    else
    {
        DUAL_DISP_EXPR(.showString("-    -"));
    }
}