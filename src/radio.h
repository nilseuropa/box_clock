#include <RCSwitch.h>

RCSwitch transmitter = RCSwitch();

class PowerSocket {
  public:
  PowerSocket(int pin_){
    this->pin=pin_;
    state = false;
  }

  void begin(){
    transmitter.enableTransmit(pin);
    transmitter.setProtocol(2);
  }

  void set(bool state_){
    if (state_) { transmitter.send("10001110010011101001101000000000"); state=true; }
    else { transmitter.send("10000001010011101001101000000000"); state=false; }
  }

  bool state;

  private:
  int pin;
};
