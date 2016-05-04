#ifndef ESP3Parser_h
#define ESP3Parser_h

#include "Arduino.h"
#include "SerialCommunication.h"

typedef void (*AfterReceivedTel)(uint8_t/*rorg*/, uint32_t/* ID */, uint32_t/* data */);

#define START_BYTE 0x55
#define RORG_RPS 0x00
#define RORG_1BS 0x01
#define RORG_4BS 0x02
#define RORG_EX_TELTYPE 0xF

class ESP3Parser
{
public:
  ESP3Parser(AfterReceivedTel pAfterReceived);
  void initialization();

private:
};

#endif // ESP3Parser_h
