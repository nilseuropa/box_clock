#ifndef BUTTON_CLASS_HEADER
#define BUTTON_CLASS_HEADER

class Button {

public:
  Button(int pin_){
    this->pin=pin_;
  }

  void begin(){
    pinMode(pin, INPUT_PULLUP);
  }

  bool pressed(){
    pin_state = digitalRead(pin);
    if (pin_state==LOW && last_state!=LOW){
      delay(50);
      last_state = pin_state;
      return true;
    }
    else {
      last_state = pin_state;
      return false;
    }
  }

  uint8_t get_state(){

    uint8_t state = 0;
    pin_state = digitalRead(pin);
    if (pin_state==LOW && last_state!=LOW){
      counter ++;
    }
    else {
      counter = 0;
    }
    last_state = pin_state;

    if (counter >= 50 && counter < 500) state = 1;
    else if (counter >= 550 ) state = 2;

    return state;
  }

private:
  int pin;
  bool pin_state = false;
  bool last_state = false;
  uint16_t counter = 0;
};

#endif
