#ifndef LED_CLASS_HEADER
#define LED_CLASS_HEADER

class Led {

public:
  Led(uint8_t pin_){
    this->pin=pin_;
  }

  void begin(){
    pinMode(pin, OUTPUT);
    this->pwm_state = 50;
    this->pwm_low   = 50;
    this->pwm_high  = 250;
    this->pulse_dir = true;
  }

  void on(){
    digitalWrite(pin, HIGH);
  }

  void off(){
    digitalWrite(pin, LOW);
  }

  void set(bool state){
    digitalWrite(pin, state);
  }

  void setBrightness(uint8_t pwm){
    analogWrite(pin, pwm);
  }

  void breathe(){
    if (pulse_dir) {
      analogWrite(pin, pwm_state);
      pwm_state++;
      if (pwm_state>=pwm_high) pulse_dir=false;
    }
    else {
      analogWrite(pin, pwm_state);
      pwm_state--;
      if (pwm_state<pwm_low) pulse_dir=true;
    }
  }

  void pulse(){
    digitalWrite(pin, !digitalRead(pin));
  }

private:
  uint8_t pin;
  uint8_t pwm_low;
  uint8_t pwm_high;
  uint8_t pwm_state;
  bool    pulse_dir;
};

#endif
