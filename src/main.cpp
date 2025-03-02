#include <Arduino.h>
#include <LedDisplay.h>
#include <timer.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <EEPROM.h>

#include "pindefs.h"
#include "button.h"
#include "radio.h"
#include "led.h"

#define WAKE_TIME_SETTING    5000
#define WAKE_TIME_CLOCK      15000
#define DEFAULT_BRIGHTNESS   5
#define ALARM_DURATION_MIN   10
#define ONE_MINUTE           60000
#define TICK                 1000
#define ALARM_HOUR_ADDRESS   1
#define ALARM_MINUTE_ADDRESS 2

LedDisplay display = LedDisplay(DISPLAY_DATA, DISPLAY_RSEL, DISPLAY_CLK,
                                  DISPLAY_CE, DISPLAY_RES, 4); // 4 characters

Button left_button(BUTTON_LEFT);
Button right_button(BUTTON_RIGHT);
PowerSocket power_socket(RADIO_TX);
Led status_led(LED_STATUS);

Timer clock_timer;
Timer led_timer;
Timer wake_timer;
Timer alarm_timer;

uint8_t op_mode = 0;
uint8_t display_brightness = DEFAULT_BRIGHTNESS;
tmElements_t time;
tmElements_t alarm_time;
uint8_t alarm_duration_minute = ALARM_DURATION_MIN;

enum OpModes{
  clock = 0,
  set_clock = 1,
  set_alarm = 2,
  sleep = 3,
  alarm = 4
};

enum ButtonEvents{
  released = 0,
  short_press = 1,
  long_press = 2
};

void load_settings(){
  alarm_time.Hour   = EEPROM.read(ALARM_HOUR_ADDRESS);
  alarm_time.Minute = EEPROM.read(ALARM_MINUTE_ADDRESS);
}

void setup() {
  left_button.begin();
  right_button.begin();
  status_led.begin();
  power_socket.begin();
  display.begin();
  display.setBrightness(display_brightness);
  display.clear();
  wake_timer.set(WAKE_TIME_CLOCK);
  load_settings();
  Serial.begin(115200);
}

void reset_time(){
  time.Hour   = 0;
  time.Minute = 0;
  time.Second = 0;
  time.Day    = 12;
  time.Month  = 9;
  time.Year = CalendarYrToTm(1981);
}

void pop_up_message_scroll(const char* message){
  display_brightness = DEFAULT_BRIGHTNESS;
  display.setBrightness(display_brightness);
  display.home();
  display.setString(message);
  for (uint8_t i=display.getCursor(); i<display.stringLength(); i++){
    display.scroll(-1);
    delay(250);
  }
}

void pop_up_message(const char* message){
  display_brightness = DEFAULT_BRIGHTNESS;
  display.setBrightness(display_brightness);
  display.home();
  display.print(message);
  delay(1000);
}

void display_clock(int h, int m){
  display.home();
  if (h<10) display.print('0');
  display.print(h);
  if (m<10) display.print('0');
  display.print(m);
}

void display_alarm(){
  display.clear();
  display.home();
  if (alarm_duration_minute>=10) display.print("T-"); else display.print(" T-");
  display.print(alarm_duration_minute);
}

void set_appliance(bool state){
  status_led.set(state);
  power_socket.set(state);
}

void reset_states(){
  display_brightness = DEFAULT_BRIGHTNESS;
  display.setBrightness(display_brightness);
  wake_timer.set(WAKE_TIME_CLOCK);
  clock_timer.set(TICK);
}

void go_to_clock(){
  left_button.reset_state();
  right_button.reset_state();
  reset_states();
  op_mode = clock;
}

void go_to_set_clock(){
  pop_up_message_scroll("   Set time");
  reset_states();
  op_mode = set_clock;
}

void go_to_set_alarm(){
  pop_up_message_scroll("   Set alarm");
  reset_states();
  op_mode = set_alarm;
}

void go_to_sleep(){
  clock_timer.set(TICK);
  display_brightness = 0;
  display.setBrightness(display_brightness);
  display.clear();
  status_led.off();
  op_mode = sleep;
}

void go_to_alarm(){
  alarm_duration_minute = ALARM_DURATION_MIN;
  alarm_timer.set(ONE_MINUTE);
  display_alarm();
  op_mode = alarm;
}

void loop() {

  switch (op_mode) {

    case sleep:
      if (clock_timer.poll(1000)){ RTC.read(time); }

      if (left_button.pressed() || right_button.pressed()) { // wake up on button press
        go_to_clock();
      }

      if (alarm_time.Hour == time.Hour && alarm_time.Minute == time.Minute) {
        set_appliance(true);
        pop_up_message_scroll("   Rise and shine!");
        go_to_alarm();
      }
      break;

    case alarm:
      if (left_button.pressed() || right_button.pressed()) {  // escape alarm mode
        set_appliance(false);
        go_to_clock();
      }

      if (alarm_timer.poll(ONE_MINUTE)){ // refresh alarm screen each minute
        alarm_duration_minute--;
        display_alarm();
      }

      if (alarm_duration_minute<=0){ // past alarm duration back to clock mode
        set_appliance(false);
        go_to_clock();
      }
      break;

    case clock:
      if (left_button.get_state()==long_press) go_to_set_clock();
      if (right_button.get_state()==long_press) go_to_set_alarm();

      if (clock_timer.poll(1000)){ // tick a second

        if (RTC.read(time)) { // display real time
          display_clock(time.Hour, time.Minute);
          status_led.pulse();
        }
        else { // can't read time off RTC
          if (RTC.chipPresent()) {
            pop_up_message_scroll("   Time is not set.");
            reset_time();
            wake_timer.set(WAKE_TIME_SETTING);
            op_mode = set_clock;
          }
          else { // RTC Error
            while (true){ // Halt and Catch Fire
              display.home();
              display.print("ERTC");
              delay(10000);
            }
          }
        }

        if (wake_timer.remaining()<DEFAULT_BRIGHTNESS*1000){ // dim the screen before going to sleep
          display_brightness--;
          if (display_brightness>=0&&display_brightness<=15){
            display.setBrightness(display_brightness);
          }
        }

        if (wake_timer.poll()){
          go_to_sleep();
        }
      }
      break;

    case set_clock:
        display_clock(time.Hour, time.Minute);

        if (left_button.pressed()) {
          time.Hour++; if (time.Hour>24) time.Hour=0;
          wake_timer.set(WAKE_TIME_SETTING);
        }

        if (right_button.pressed()) {
          time.Minute++; if (time.Minute>60) time.Minute=0;
          wake_timer.set(WAKE_TIME_SETTING);
        }

        if (wake_timer.poll()) { // time's up for settings, saving time
          RTC.write(time);
          pop_up_message("SAVE");
          go_to_clock();
        }

        if (led_timer.poll(100)) status_led.pulse();
        break;

    case set_alarm:
      display_clock(alarm_time.Hour, alarm_time.Minute);

      if (left_button.pressed()) {
        alarm_time.Hour++; if (alarm_time.Hour>24) alarm_time.Hour=0;
        wake_timer.set(WAKE_TIME_SETTING);
      }

      if (right_button.pressed()) {
        alarm_time.Minute++; if (alarm_time.Minute>60) alarm_time.Minute=0;
        wake_timer.set(WAKE_TIME_SETTING);
      }

      if (wake_timer.poll()) {
        EEPROM.update(ALARM_HOUR_ADDRESS, alarm_time.Hour);
        EEPROM.update(ALARM_MINUTE_ADDRESS, alarm_time.Minute);
        pop_up_message("SAVE");
        go_to_clock();
      }

      if (led_timer.poll(100)) status_led.pulse();
      break;

    default:
      go_to_clock();
      break;
  }


}
