#ifndef Pins_h
#define Pins_h

#define P_ROTA 0
#define P_ROTB 1
#define P_ROTP 2

#define P_DIO 0
#define P_CLK 1

const constexpr struct
{
	const byte
		displays[2][2] = { { 7, 8 }, { 5, 6 } },
		rotary[3] = { 2, 3, 4 };
} pins;

#endif