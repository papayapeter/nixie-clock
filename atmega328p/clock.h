#ifndef CLOCK_H
#define CLOCK_H

// libraries -------------------------------------------------------------------
#include <Bounce2.h>
#include <Shifty.h>
#include <MCP7940.h>

// class -----------------------------------------------------------------------
class Clock
{
  private:
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

    uint8_t table[11][4];
  public:
    /**
     names for button states
    */
    const uint8_t pressed_1;
    const uint8_t pressed_2;
    const uint8_t pressed_3;

    /**
     @brief   creates and instance of clock
    */
    Clock() : pressed_1(1), pressed_2(2), pressed_3(3) { };

    /**
     sets all the pins and debounced switches and connects with rtc

     @ param  debounce  sets debounce time for buttons in milliseconds

     @ param  tube_table  requires a [11][4] array which tells the function
                          which bits to push for which number
    */
    void init(uint8_t data, uint8_t clock, uint8_t latch, uint8_t dot,
              uint8_t switch_1, uint8_t switch_2, uint8_t switch_3,
              uint16_t debounce, uint8_t tube_table[][4]);
    /**
     @brief   fetches time from the rtc

     @return  return true if time has changed
    */
    bool updateTime();
    /**
     @brief   sets time on the rtc
    */
    void setTime(uint8_t hour, uint8_t minute);
    /**
     @brief   returns the current minute stored
    */
    uint8_t getMinute();
    /**
     @brief   returns the current hour stored
    */
    uint8_t getHour();
    /**
     @brief   checks for button presses

     @return  returns name for button states (or 0 if nothing was pressed)
    */
    uint8_t updateButtons();
    /**
     @brief   checks if the current second is an even number
    */
    bool evenSecond();
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
};

#endif
