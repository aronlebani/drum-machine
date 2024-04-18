// drum_machine.ino
//
// Mini set-and-forget drum machine for guitar practice with a touchscreen
// interface.
// (c) Aron Lebani 2024 

#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <TimerOne.h>

#define _DEBUG

// Pins
#define XP 8
#define XM A2
#define YP A3
#define YM 9

// Calibration data
#define TS_LEFT 912
#define TS_RT 104
#define TS_TOP 75
#define TS_BOT 901

// Parameters
#define MINPRESSURE 200
#define MAXPRESSURE 1000
#define DEBOUNCE_DELAY 100
#define ORIENTATION 1	// Landscape

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Globals
MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;

uint16_t box_width;
uint16_t box_height;
uint16_t id;
uint16_t bpm = 94;
uint16_t xpos;
uint16_t ypos;

uint64_t last_toggled = 0;

uint8_t previous_current_beat = 0;
volatile uint8_t current_beat = 0;

uint8_t slots[6][16] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
};

// ---- Initialization functions ----

void initialize_serial(void) {
	#ifdef _DEBUG
	Serial.begin(9600);
	#endif
}

void initialize_display(void) {
    id = tft.readID();

    tft.reset();
    tft.begin(id);
    tft.setRotation(ORIENTATION);
    tft.fillScreen(BLACK);

    box_width = tft.width() / 16;
	box_height = tft.height() / 6;

    tft.fillScreen(BLACK);
}

void initialize_timer(void) {
	// Divide by 4 because we're using 16th notes
	Timer1.initialize(1000000 * 60 / bpm / 4);
	Timer1.attachInterrupt(progress_beat);
}

// ---- Drawing functions ----

void draw_screen(void) {
	for (int beat = 0; beat < 16; beat++) {
		draw_beat(beat);
	}
}

void draw_beat(int beat) {
	for (int drum = 0; drum < 6; drum++) {
		draw_slot(drum, beat);
	}
}

void draw_slot(int drum, int beat) {
	int x = beat * box_width;
	int y = drum * box_height;

	tft.fillRect(x + 1, y + 1, box_width - 2, box_height - 2, BLACK);

	if (slots[drum][beat] == 1) {
		tft.fillRect(x + 1, y + 1, box_width - 2, box_height - 2, BLUE);
	} else {
		tft.drawRect(x + 1, y + 1, box_width - 2, box_height - 2, BLUE);
	}
}

// ---- State manipulation ----

void toggle_slot(int drum, int beat) {
	if (millis() - last_toggled < DEBOUNCE_DELAY) {
		return;
	}

	last_toggled = millis();

	if (slots[drum][beat] == 1) {
		slots[drum][beat] = 0;
	} else {
		slots[drum][beat] = 1;
	}

	draw_slot(drum, beat);
}

void progress_beat(void) {
	if (current_beat == 15) {
		current_beat = 0;
		return;
	}

	current_beat = current_beat + 1;
}

uint8_t get_previous_beat(void) {
	if (current_beat == 0) {
		return 15;
	}

	return current_beat - 1;
}

void clear_previous_beat(void) {
	uint8_t previous_beat = get_previous_beat();

	draw_beat(previous_beat);
}

// ---- Event loop ----

void highlight_current_beat(void) {
	if (current_beat == previous_current_beat) {
		return;
	}

	previous_current_beat = current_beat;

	clear_previous_beat();

	for (int drum = 0; drum < 6; drum++) {
		int x = current_beat * box_width;
		int y = drum * box_height;

		tft.drawRect(x + 1, y + 1, box_width - 2, box_height - 2, WHITE);
	}
}

void wait_for_touch(void) {
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

// ---- Entry point ----

void setup(void) {
	initialize_serial();
	initialize_display();
	initialize_timer();

	draw_screen();
}

void loop() {
	highlight_current_beat();
	wait_for_touch();
}
