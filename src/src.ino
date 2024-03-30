// drum_machine.ino
//
// Mini set-and-forget drum machine for guitar practice with a touchscreen
// interface.
// (c) Aron Lebani 2024 

#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>

#define _DEBUG

const int XP=8,XM=A2,YP=A3,YM=9;	// ID = 0x7575
const int TS_LEFT=912,TS_RT=104,TS_TOP=75,TS_BOT=901;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;

#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define DEBOUNCE_DELAY 100

uint16_t BOXWIDTH;
uint16_t BOXHEIGHT;
uint16_t ID;
const uint8_t ORIENTATION = 1;	// LANDSCAPE

uint16_t xpos;
uint16_t ypos;

uint8_t slots[6][16] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
};

uint64_t last_toggled = 0;

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

void initialise_slots(MCUFRIEND_kbv tft) {
	for (int drum = 0; drum < 6; drum++) {
		for (int beat = 0; beat < 16; beat++) {
			int x = beat * BOXWIDTH;
			int y = drum * BOXHEIGHT;
			
			if (slots[drum][beat] == 1) {
				tft.fillRect(x + 1, y + 1, BOXWIDTH - 2, BOXHEIGHT - 2, BLUE);
			} else {
				tft.drawRect(x + 1, y + 1, BOXWIDTH - 2, BOXHEIGHT - 2, BLUE);
			}
		}
	}
}

void toggle_slot(MCUFRIEND_kbv tft, int drum, int beat) {
	if (millis() - last_toggled < DEBOUNCE_DELAY) {
		return;
	}

	last_toggled = millis();

	int x = beat * BOXWIDTH;
	int y = drum * BOXHEIGHT;	

	tft.fillRect(x + 1, y + 1, BOXWIDTH - 2, BOXHEIGHT - 2, BLACK);

	if (slots[drum][beat] == 1) {
		slots[drum][beat] = 0;
		tft.drawRect(x + 1, y + 1, BOXWIDTH - 2, BOXHEIGHT - 2, BLUE);
	} else {
		slots[drum][beat] = 1;
		tft.fillRect(x + 1, y + 1, BOXWIDTH - 2, BOXHEIGHT - 2, BLUE);
	}
}

void setup(void) {
	#ifdef _DEBUG
	Serial.begin(9600);
	#endif

    tft.reset();
    ID = tft.readID();
    tft.begin(ID);
    tft.setRotation(ORIENTATION);
    tft.fillScreen(BLACK);

    BOXWIDTH = tft.width() / 16;
	BOXHEIGHT = tft.height() / 6;
    tft.fillScreen(BLACK);

	initialise_slots(tft);
}

void loop() {
    tp = ts.getPoint();

    // If sharing pins, need to fix the directions of the touchscreen pins
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

	if (tp.z < MINPRESSURE || tp.z > MAXPRESSURE) {
		return;
	}

	// Map the tp coordinates based on the calibration data and orientation
    xpos = map(tp.y, TS_TOP, TS_BOT, 0, tft.width());
    ypos = map(tp.x, TS_RT, TS_LEFT, 0, tft.height());

	int beat = xpos / BOXWIDTH;
	int drum = ypos / BOXHEIGHT;
	toggle_slot(tft, drum, beat);
}
