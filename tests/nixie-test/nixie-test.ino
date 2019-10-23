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
Metro counter(1000);

Clock clock;

// variables -------------------------------------------------------------------
uint8_t count = 0;

// setup -----------------------------------------------------------------------
void setup()
{
  //Serial.begin(9600);

  clock.init(DATA, CLOCK, LATCH, DOT,
             SWITCH_1, SWITCH_2, SWITCH_3,
             debounce_delay, table);
}

// loop ------------------------------------------------------------------------
void loop()
{
  // count from 0 to 99 in a loop
  if (counter.check())
  {
    clock.display(count, count);

    if (++count > 99)
      count = 0;
  }
}
