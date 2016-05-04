/*
  Simple Display

  This sketch displays the data of EnOcean radio telegram to serial console
  using an EnOcean Shield (TCM410J) by SiMICS.

  The circuit:
  *Input PIN
    RX:EnOcean (TCM410J)
  *Output PIN
    None

  Created 1 May 2016
  by LoonaiFactory

  https://github.com/simics-co/EnOcean
*/

#include "ESP3Parser.h"

ESP3Parser parser(NULL);


void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  parser.initialization();
}

void loop()
{
  ;
}
