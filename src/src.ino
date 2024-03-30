// drum_machine.ino
//
// Mini set-and-forget drum machine for guitar practice with a touchscreen
// interface.
// (c) Aron Lebani 2024 

#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>

#include "TimerOne.h"

#define _DEBUG

const int XP=8,XM=A2,YP=A3,YM=9;	// ID = 0x7575
const int TS_LEFT=912,TS_RT=104,TS_TOP=75,TS_BOT=901;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;

#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define DEBOUNCE_DELAY 100

const uint8_t ORIENTATION = 1;	// LANDSCAPE
uint16_t box_width;
uint16_t box_height;
uint16_t id;
uint16_t bpm = 94;
uint16_t xpos;
uint16_t ypos;

volatile uint8_t current_beat = 0;

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

void progress_beat(void) {
	if (current_beat == 15) {
		current_beat = 0;
		return;
	}

	current_beat = current_beat + 1;
}

void initialize_timer(void) {
	Timer1.initialize(1000000 * 60 / bpm);
	Timer1.attachInterrupt(progress_beat);
}

void initialize_slots() {
	for (int drum = 0; drum < 6; drum++) {
		for (int beat = 0; beat < 16; beat++) {
			int x = beat * box_width;
			int y = drum * box_height;
			
			if (slots[drum][beat] == 1) {
				tft.fillRect(x + 1, y + 1, box_width - 2, box_height - 2, BLUE);
			} else {
				tft.drawRect(x + 1, y + 1, box_width - 2, box_height - 2, BLUE);
			}
		}
	}
}

void toggle_slot(int drum, int beat) {
	if (millis() - last_toggled < DEBOUNCE_DELAY) {
		return;
	}

	last_toggled = millis();

	int x = beat * box_width;
	int y = drum * box_height;

	tft.fillRect(x + 1, y + 1, box_width - 2, box_height - 2, BLACK);

	if (slots[drum][beat] == 1) {
		slots[drum][beat] = 0;
		tft.drawRect(x + 1, y + 1, box_width - 2, box_height - 2, BLUE);
	} else {
		slots[drum][beat] = 1;
		tft.fillRect(x + 1, y + 1, box_width - 2, box_height - 2, BLUE);
	}
}

uint8_t get_previous_beat(void) {
	if (current_beat == 0) {
		return 15;
	}

	return current_beat - 1;
}

void clear_previous_beat(void) {
	uint8_t previous_beat = get_previous_beat();

	Serial.println("current-beat: " + String(current_beat));
	Serial.println("prev-beat: " + String(previous_beat));

	for (int drum = 0; drum < 6; drum++) {
		int x = previous_beat * box_width;
		int y = drum * box_height;

		if (slots[drum][previous_beat] == 1) {
			tft.fillRect(x + 1, y + 1, box_width - 2, box_height - 2, BLUE);
		} else {
			tft.drawRect(x + 1, y + 1, box_width - 2, box_height - 2, BLUE);
		}
	}
}

void highlight_current_beat(void) {
	clear_previous_beat();

	for (int drum = 0; drum < 6; drum++) {
		int x = current_beat * box_width;
		int y = drum * box_height;

		tft.drawRect(x + 1, y + 1, box_width - 2, box_height - 2, WHITE);
	}
}

void setup(void) {
	#ifdef _DEBUG
	Serial.begin(9600);
	#endif

    tft.reset();
    id = tft.readID();
    tft.begin(id);
    tft.setRotation(ORIENTATION);
    tft.fillScreen(BLACK);

    box_width = tft.width() / 16;
	box_height = tft.height() / 6;
    tft.fillScreen(BLACK);

	initialize_slots();
	initialize_timer();
}

void loop() {
	highlight_current_beat();

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

	int beat = xpos / box_width;
	int drum = ypos / box_height;
	toggle_slot(drum, beat);
}
