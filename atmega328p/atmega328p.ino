minute_on// nixie clock r1.1
//
// zeno gries

// makros ----------------------------------------------------------------------
#define DEBUG

// libraries -------------------------------------------------------------------
#include <MCP7940.h>
#include <Metro.h>
#include <Bounce2.h>
#include <Shifty.h>

// settings -------------------------------------------------------------------
uint64_t debounce_delay = 100;

// pins ------------------------------------------------------------------------
const uint8_t DATA  = 2;
const uint8_t CLOCK = 3;
const uint8_t LATCH = 4;

const uint8_t DOT = 5;

const uint8_t SWITCH_1 = 6; // right
const uint8_t SWITCH_2 = 7; // left up
const uint8_t SWITCH_3 = 8; // left down

// classes ---------------------------------------------------------------------
class Clock
{
    // pins
    uint8_t pin_dot;

    // objects
    Shifty shift;
    MCP7940_Class real_time;

    DateTime now;
    DateTime last;

    Bounce button_1;
    Bounce button_2;
    Bounce button_3;
  public:
    /**
     names for button states
    */
    const uint8_t pressed_1;
    const uint8_t pressed_2;
    const uint8_t pressed_3;

    /**
     @brief   creates and instance of clock

     sets all the pins and debounced switches and connects with rtc

     @ param  debounce  sets debounce time for buttons in milliseconds
    */
    Clock(uint8_t data, uint8_t clock, uint8_t latch, uint8_t dot,
          uint8_t switch_1, uint8_t switch_2, uint8_t switch_3,
          uint16_t debounce);
    /**
     @brief   fetches time from the rtc
    */
    bool updateTime();
    /**
     @brief   sets time on the rtc
    */
    void setTime(uint8_t hour, uint8_t minute)
    /**
     @brief   checks for button presses

     @return  returns name for button states (or 0 if nothing was pressed)
    */
    uint8_t updateButtons();
    /**
     @brief   sets dot on or off
    */
    void setDot(bool state);
    /**
     @brief   displays time on nixie tubes

     uses time from the now object in the class
    */
    bool display();
    /**
     @brief   displays time on nixie tubes

     @params  hour_on, minute_on  turns tubes for hours or minutes on or off

     @return  returns false if hour or minute out of bounds
    */
    bool display(uint8_t hour, uint8_t minute,
                 bool hour_on = true, bool minute_on = true);
}

// objects ---------------------------------------------------------------------
Metro debug_time(100);

Clock clock(DATA, CLOCK, LATCH, DOT, SWITCH_1, SWITCH_2, SWITCH_3, debounce_delay);

// variables -------------------------------------------------------------------

// setup -----------------------------------------------------------------------
void setup()
{

}

// loop ------------------------------------------------------------------------
void loop()
{

}

// methods ---------------------------------------------------------------------
Clock::Clock(uint8_t data, uint8_t clock, uint8_t latch, uint8_t dot,
      uint8_t switch_1, uint8_t switch_2, uint8_t switch_3, uint16_t debounce)
      : pin_dot(dot), pressed_1(1), pressed_2(2), pressed_3(3)
{
  // debug
  #ifdef DEBUG
    Serial.begin(9600);
  #endif

  // set pin mode
  pinMode(pin_dot, OUTPUT);

  // set debounced buttons
  button_1.attach(switch_1, INPUT_PULLUP);
  button_2.attach(switch_2, INPUT_PULLUP);
  button_3.attach(switch_3, INPUT_PULLUP);
  button_1.interval(debounce);
  button_2.interval(debounce);
  button_3.interval(debounce);

  // initialize shift registers
  shift.setBitCount(16);
  shift.setPins(data, clock, latch);

  // set diplay to zero with dot on
  this->display(0, 0);
  digitalWrite(pin_dot, HIGH);

  // begin communication with rtc and start oscillator
  while (!real_time.begin())
  {
    // if cannot connect to rtc -> blink fast
    digitalWrite(pin_dot, HIGH);
    delay(500);
    digitalWrite(pin_dot, LOW);
    delay(500);

    // debug
    #ifdef DEBUG
      Serial.println("cannot connect. reattempting...");
    #endif
  }
  // if connected -> turn dot on and start rtc
  real_time.deviceStart();
  digitalWrite(pin_dot, HIGH);

  // debug
  #ifdef DEBUG
    Serial.println("connected");
  #endif

  // update and display the time
  this->update();
  this->display();
}

bool Clock::updateTime()
{
  // get time from rtc
  now = real_time.now();

  // debug
  #ifdef DEBUG
    Serial.println("H: " + String(now.hour()) +
                 "\tM: " + String(now.minute()) +
                 "\tS: " + String(now.second()));
  #endif

  // return true if time has changed
  if (now.minute() != last.minute() || now.hour() != last.hour()) // if time changed
  {
    last = now;
    return true;
  }

  return false;
}

void Clock::setTime(uint8_t hour, uint8_t minute)
{
  real_time.adjust(DateTime(2019, 3, 23, hour, minute, 0));
}

uint8_t Clock::updateButtons()
{
  button_1.update();
  button_2.update();
  button_3.update();

  if (button_1.rose() || button_1.fell()) return pressed_1;
  if (button_2.rose() || button_2.fell()) return pressed_2;
  if (button_3.rose() || button_3.fell()) return pressed_3;
  return 0;
}

void Clock::setDot(bool state)
{
  if (state) digitalWrite(pin_dot, HIGH);
  else       digitalWrite(pin_dot, LOW);
}

bool Clock::display()
{
  return this->display(now.hour(), now.minute(), true, true);
}

bool Clock::display(uint8_t hour, uint8_t minute,
                    bool hour_on = true, bool minute_on = true)
{
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

  // test out of bounds
  if (hour > 99 || minute > 99)
  {
    return false;
  }

  uint8_t h_1;
  uint8_t h_2;
  uint8_t m_1;
  uint8_t m_2;
  // get single digits or turn display off
  if (hour_on)
  {
    h_1 = hour / 10;
    h_2 = hour % 10;
  }
  else
  {
    h_1 = 10;
    h_2 = 10;
  }
  if (minute_on)
  {
    m_1 = minute / 10;
    m_2 = minute % 10;
  }
  else
  {
    m_1 = 10;
    m_2 = 10;
  }

  shift.batchWriteBegin();

  // m1
  shift.writeBit(0, table[m_1][0]);
  shift.writeBit(1, table[m_1][1]);
  shift.writeBit(2, table[m_1][2]);
  shift.writeBit(3, table[m_1][3]);

  // m2
  shift.writeBit(4, table[m_2][0]);
  shift.writeBit(5, table[m_2][1]);
  shift.writeBit(6, table[m_2][2]);
  shift.writeBit(7, table[m_2][3]);

  // h1
  shift.writeBit(8, table[h_1][0]);
  shift.writeBit(9, table[h_1][1]);
  shift.writeBit(10, table[h_1][2]);
  shift.writeBit(11, table[h_1][3]);

  // h2
  shift.writeBit(12, table[h_2][0]);
  shift.writeBit(13, table[h_2][1]);
  shift.writeBit(14, table[h_2][2]);
  shift.writeBit(15, table[h_2][3]);

  shift.batchWriteEnd();

  return true;
}
