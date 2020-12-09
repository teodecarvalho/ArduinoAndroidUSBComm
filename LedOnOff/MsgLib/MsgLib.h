#ifndef msglib
#define msglib

#if(ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class MsgLib {
  public:
    // Constructor
    MsgLib(int msgSize = 20);

    // Methods
    void begin(int baudRate = 9600);
    void receiveMsg();
    void clearMsg();

    // Variables
    char msg[20];

  private:
    int _msgSize;
};

#endif
