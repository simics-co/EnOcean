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
  Modified (Added support for ESP8266) 12 July 2016
  by LoonaiFactory

  https://github.com/simics-co/EnOcean
*/

#ifndef ESP8266
  #include <avr/io.h>
  #include <avr/interrupt.h>
#endif

#include "Arduino.h"
#include "SerialCommunication.h"

static uint8_t empty(char aChar)
{
  return 0;
}

const ReceptionOpe dummySet[] = {
  empty
};

static ReceptionOpe* pReceptOpeSet = (ReceptionOpe*)dummySet;
static uint8_t state = 0;

#ifdef ESP8266
  static void uart0_rx_intr_handler(void *para)
  {
    if (!(USIS(UART0) & 0x1c)) {
      while(USS(UART0) & 0x00ff) {
        unsigned char c = USF(UART0) & 0xff;
        state = (pReceptOpeSet[state])(c);
      }
      
    } else {
      // Overflow/Frame/Parity error, read byte but discard it
      unsigned char c = USF(UART0) & 0xff;
      digitalWrite(14, 1);
    }
    USIC((UART0)) = 0xff; /* Clear interrupt factor */
  }

#else

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
#endif

void SerialCommunication::Initialization(void)
{
  volatile unsigned char c;

#ifdef ESP8266
  pinMode(14, OUTPUT);
  digitalWrite(14, 0);
  
  USIE(UART0) = 0x00; /* Disable interrupt */
  USC1(UART0) = 1;    /* Rx fifo full threshold is 1 */
  
  while((USS(UART0) >> USRXC) & 0xff) {
    c = USF(UART0) & 0xff;
  }
  ETS_UART_INTR_ATTACH(uart0_rx_intr_handler, NULL);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);
  ETS_UART_INTR_ENABLE();
  
  USIC((UART0)) = 0xff; /* Clear interrupt factor */
  USIE(UART0) = (0x01 << UIFF); /* Enable interrupt for rx fifo full */

#else
  #define RXCn 0x80
  
  while(UCSR0A & RXCn) {
    c = UDR0;
  }
#endif

  state = 0;
}

void SerialCommunication::SetReceptOpe(ReceptionOpe* pRcvOpeSet)
{
  pReceptOpeSet = pRcvOpeSet;
}
