/*
  Serial Communication

  This sketch gets a serial byte data in interrupt processing.
  If ISR(USART_RX_vect) is written in HardwareSerial0.cpp, comment out it.

  The circuit:
  *Input PIN
    RX
  *Output PIN
    None

  Created 1 May 2016
  by LoonaiFactory

  https://github.com/simics-co/EnOcean
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Arduino.h"
#include "SerialCommunication.h"

#define RXCn 0x80

static uint8_t empty(char aChar)
{
  return 0;
}

const ReceptionOpe dummySet[] = {
  empty
};

static ReceptionOpe* pReceptOpeSet = (ReceptionOpe*)dummySet;
static uint8_t state = 0;


#if defined(USART_RX_vect)
  ISR(USART_RX_vect)
  
#elif defined(USART0_RX_vect)
  ISR(USART0_RX_vect)
  
#elif defined(USART_RXC_vect)
  ISR(USART_RXC_vect) // ATmega8
  
#else
  #error "Don't know what the Data Received vector is called for Serial"
#endif

  {
    if (bit_is_clear(UCSR0A, UPE0)) {
      unsigned char c = UDR0;
      state = (pReceptOpeSet[state])(c);
      
    } else {
      // Parity error, read byte but discard it
      unsigned char c = UDR0;
    }	
  }

void SerialCommunication::Initialization(void)
{
  volatile unsigned char c;
  while(UCSR0A & RXCn) {
    c = UDR0;
  }
  state = 0;
}

void SerialCommunication::SetReceptOpe(ReceptionOpe* pRcvOpeSet)
{
  pReceptOpeSet = pRcvOpeSet;
}
