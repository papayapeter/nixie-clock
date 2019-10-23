// nixie clock r1.1
//
// zeno gries

// libraries -------------------------------------------------------------------
#include <Metro.h>
#include "clock.h"

// settings -------------------------------------------------------------------
uint64_t debounce_delay = 100;

// bit table for tube driver
const uint8_t table[11][4] = {{0, 0, 0, 0},  // 0
                              {1, 0, 0, 1},  // 1
                              {0, 0, 0, 1},  // 2
                              {1, 1, 1, 0},  // 3
                              {0, 1, 1, 0},  // 4
                              {1, 0, 1, 0},  // 5
                              {0, 0, 1, 0},  // 6
                              {1, 1, 0, 0},  // 7
                              {0, 1, 0, 0},  // 8
                              {1, 0, 0, 0},  // 9
                              {1, 1, 1, 1}}; // NONE

// pins ------------------------------------------------------------------------
const uint8_t DATA  = 2;
const uint8_t CLOCK = 3;
const uint8_t LATCH = 4;

const uint8_t DOT = 5;

const uint8_t SWITCH_1 = 6; // right
const uint8_t SWITCH_2 = 7; // left up
const uint8_t SWITCH_3 = 8; // left down

// objects ---------------------------------------------------------------------
Metro time(10);
Metro dot(10);
Metro buttons(5);

Clock clock;

// variables -------------------------------------------------------------------
const uint8_t STATE_NORMAL      = 0;
const uint8_t STATE_CLOCK_OFF   = 1;
const uint8_t STATE_SET_HOUR    = 2;
const uint8_t STATE_SET_MINUTE  = 3;

uint8_t state = STATE_NORMAL;

bool first_on     = true;
bool first_off    = false;
bool first_hour   = false;
bool first_minute = false;

uint8_t set_hour;
uint8_t set_minute;

// setup -----------------------------------------------------------------------
void setup()
{
  Serial.begin(9600);

  clock.init(DATA, CLOCK, LATCH, DOT,
             SWITCH_1, SWITCH_2, SWITCH_3,
             debounce_delay, table);
}

// loop ------------------------------------------------------------------------
void loop()
{
  // normal clock reading and updating -----------------------------------------
  if (state == STATE_NORMAL)
  {
    if (first_on) // if on for the first time -> update and display
    {
      clock.updateTime();
      clock.display();
      first_on = false;
    }
    // check time and update tubes
    if (time.check())
    {
      if (clock.updateTime())
        clock.display();
    }
    // blink dot every second
    if (dot.check())
    {
      clock.setDot(clock.evenSecond());
    }
    // check buttons
    if (buttons.check())
    {
      uint8_t button_state = clock.updateButtons();
      if (button_state == clock.pressed_2)
      {
        state = STATE_CLOCK_OFF;
        first_off = true;
      }
      else if (button_state == clock.pressed_1)
      {
        state = STATE_SET_HOUR;
        first_hour = true;
      }
    }
  }
  // clock off -----------------------------------------------------------------
  else if (state == STATE_CLOCK_OFF)
  {
    if (first_off) // if off for the first time -> turn off
    {
      clock.display(0, 0, false, false);
      clock.setDot(false);
      first_off = false;
    }
    // check buttons
    if (buttons.check())
    {
      uint8_t button_state = clock.updateButtons();
      if (button_state == clock.pressed_2)
      {
        state = STATE_NORMAL;
        first_on = true;
      }
    }
  }
  // set hour ------------------------------------------------------------------
  else if (state == STATE_SET_HOUR)
  {
    if (first_hour) // if on set hour for the first time
    {
      set_hour = clock.getHour();
      clock.display(set_hour, 0, true, false);
      first_hour = false;
    }
    // blink dot every second
    if (dot.check())
    {
      clock.setDot(clock.evenSecond());
    }
    // check buttons
    if (buttons.check())
    {
      uint8_t button_state = clock.updateButtons();
      // up
      if (button_state == clock.pressed_2)
      {
        if (++set_hour > 24) set_hour = 0;
        clock.display(set_hour, 0, true, false);
      }
      // down
      if (button_state == clock.pressed_3)
      {
        if (--set_hour < 0) set_hour = 24;
        clock.display(set_hour, 0, true, false);
      }
      else if (button_state == clock.pressed_1)
      {
        state = STATE_SET_MINUTE;
        first_minute = true;
      }
    }
  }
  // set minute ----------------------------------------------------------------
  else if (state == STATE_SET_MINUTE)
  {
    if (first_minute) // if on set minute for the first time
    {
      set_minute = clock.getMinute();
      clock.display(0, set_minute, false, true);
      first_minute = false;
    }
    // blink dot every second
    if (dot.check())
    {
      clock.setDot(clock.evenSecond());
    }
    // check buttons
    if (buttons.check())
    {
      uint8_t button_state = clock.updateButtons();
      // up
      if (button_state == clock.pressed_2)
      {
        if (++set_minute > 60) set_minute = 0;
        clock.display(0, set_minute, false, true);
      }
      // down
      if (button_state == clock.pressed_3)
      {
        if (--set_minute < 0) set_minute = 24;
        clock.display(0, set_minute, false, true);
      }
      else if (button_state == clock.pressed_1)
      {
        clock.setTime(set_hour, set_minute);
        state = STATE_NORMAL;
        first_on = true;
      }
    }
  }
}
