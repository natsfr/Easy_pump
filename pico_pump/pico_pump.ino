#include "RPi_Pico_TimerInterrupt.h"

#include "RPi_Pico_ISR_Timer.h"

#include <EEPROM.h>

// Init RPI_PICO_Timer
RPI_PICO_Timer ITimer1(1);

RPI_PICO_ISR_Timer ISR_timer;

#ifndef LED_BUILTIN
  #define LED_BUILTIN       25
#endif

#define PUMP_IO             1

#define LED_TOGGLE_INTERVAL_MS        1000L
#define TIMER_INTERVAL_MS             1L
#define TIMER_FREQ_HZ                 1000L

bool TimerHandler(struct repeating_timer *t)
{

  ISR_timer.run();

  return true;
}

bool run_pump = false;
bool save_counter = false;
uint32_t time_trigger = 172800; // 48 hours in seconds
uint32_t time_counter = 0; // 1 second increment
uint8_t hours_count = 0;
uint32_t hours_edge = 0;
uint32_t address = 0;

void increment_time() {
  static bool toggle  = false;

  digitalWrite(LED_BUILTIN, toggle);
  toggle = !toggle;

  time_counter += 10;
  hours_edge += 10;

  if (time_counter >= 172800) {
    run_pump = true;
    time_counter = 0;
    hours_count = 0;
    hours_edge = 0;
    save_counter = true;
  } else if (hours_edge >= 3600) {
    hours_count++;
    hours_edge = 0;
    save_counter = true;
  }
}

void write_to_flash() {
  EEPROM.write(address, hours_count);
  EEPROM.commit();
}

void delay_run_pump() {
  digitalWrite(PUMP_IO, 1);
  delay(30000);
  digitalWrite(PUMP_IO, 0);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 0);
    
    pinMode(PUMP_IO, OUTPUT);
    digitalWrite(PUMP_IO, 0);

    /*
     * We try to restore hours counter in case of previous failure.
     */
    EEPROM.begin(256);
    uint8_t value = EEPROM.read(address);

    time_counter = value * 3600;

    ITimer1.attachInterruptInterval(TIMER_FREQ_HZ, TimerHandler);

    ISR_timer.setInterval(10000L, increment_time);
}

void loop() {
    if(save_counter) {
      write_to_flash();
      save_counter = false;
    }
    if(run_pump) {
      delay_run_pump();
      run_pump = false;
    }
}
