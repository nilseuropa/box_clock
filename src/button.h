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

    state = 0; // not pressed

    pin_state = digitalRead(pin);

    if (pin_state==LOW){ // button is pressed increment counter
      counter ++;
      if (counter > 25000 ) { // escape at long press
        counter = 0;
        return 2;
      }
    }
    else if (pin_state==HIGH && last_state==LOW) { // button released
      if (counter >= 50 && counter < 20000) state = 1; // short press
      counter = 0;
    }
    last_state = pin_state;

    return state;
  }

  void reset_state(){
    state = 0;
  }

  uint8_t state    = 0;

private:
  int pin;
  bool pin_state   = false;
  bool last_state  = false;
  uint16_t counter = 0;
};

#endif
