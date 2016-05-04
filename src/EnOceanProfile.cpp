/*
  EnOcean Profile

  This sketch converts the data of EnOcean radio telegram to the specific value.

  The circuit:
  *Input PIN
    None
  *Output PIN
    None

  Created 1 May 2016
  by LoonaiFactory

  https://github.com/simics-co/EnOcean
*/

#include "Arduino.h"
#include "EnOceanProfile.h"

//!     07-02-05 temperature maximum temperature
#define EEP_A5_02_05_T_MAX      (40.0)
//!     07-02-05 temperature minumum temperature
#define EEP_A5_02_05_T_MIN      (0.0)
//!     07-02-05 data for T=0°C
#define EEP_A5_02_05_T0_DATA    (255.0)
//!     07-02-05 data for T=40°C
#define EEP_A5_02_05_T40_DATA   (0.0)
//!     07-02-05 temperature scaling factor
#define EEP_A5_02_05_T_SCALE    (-40.0/255.0)

//!     07-02-30 temperature maximum temperature
#define EEP_A5_02_30_T_MAX      (62.3)
//!     07-02-30 temperature minumum temperature
#define EEP_A5_02_30_T_MIN      (-40.0)
//!     07-02-30 data for T=0°C
#define EEP_A5_02_30_T_40_DATA  (1023.0)
//!     07-02-30 data for T=40°C
#define EEP_A5_02_30_T62_3_DATA (0.0)
//!     07-02-30 temperature scaling factor
#define EEP_A5_02_30_T_SCALE   ((EEP_A5_02_30_T_MAX - EEP_A5_02_30_T_MIN) / (EEP_A5_02_30_T62_3_DATA - EEP_A5_02_30_T_40_DATA))

//!     07-08-02 temperature maximum temperature
#define EEP_A5_08_02_T_MAX      (51.0)
//!     07-08-02 temperature minumum temperature
#define EEP_A5_08_02_T_MIN      (0.0)
#define EEP_A5_08_02_T0_DATA    (0.0)
#define EEP_A5_08_02_T51_DATA   (255.0)

#define PIR_OFF 0
#define PIR_ON  1


uint8_t EnOceanProfile::getSwitchStatus(EEP_TYPE type, uint32_t dataRPS)
{
  uint8_t state;
  
  switch(type) {
    case EEP_F6_02_04:
      state = (uint8_t)((dataRPS >> 24) & 0xFF);
      break;
      
    default:
      break;
  }
  
  return state;
}

uint8_t EnOceanProfile::getContact(EEP_TYPE type, uint32_t data1BS)
{
  uint8_t contact;
  
  switch(type) {
    case EEP_D5_00_01:
      contact = (uint8_t)((data1BS >> 24) & 0x01);
      break;
      
    default:
      break;
  }
  
  return contact;
}

float EnOceanProfile::getTemperature(EEP_TYPE type, uint32_t data4BS)
{
  uint16_t temp;
  float temperature;
  
  switch(type) {
    case EEP_A5_02_05:
      temp = (uint16_t)((data4BS >> 8) & 0x00FF);
      temperature = (float)(temp * EEP_A5_02_05_T_SCALE) - (float)(EEP_A5_02_05_T0_DATA * EEP_A5_02_05_T_SCALE) + EEP_A5_02_05_T_MIN;
      break;
        
    case EEP_A5_02_30:
      temp = (uint16_t)((data4BS >> 8) & 0x03FF);
      temperature = (float)(temp * EEP_A5_02_30_T_SCALE) - (float)(EEP_A5_02_30_T_40_DATA * EEP_A5_02_30_T_SCALE) + EEP_A5_02_30_T_MIN;
      break;
      
    case EEP_A5_08_02:
      temp = (uint16_t)((data4BS >> 8) & 0x00FF);
      temperature = (float)(temp * EEP_A5_08_02_T_MAX / EEP_A5_08_02_T51_DATA);
      break;
        
    default:
      break;
  }
  
  return temperature;
}

uint8_t EnOceanProfile::getPIRStatus(EEP_TYPE type, uint32_t data4BS)
{
  uint8_t pir;
  
  switch(type) {
    case EEP_A5_07_01:
      if(((data4BS >> 8) & 0xFF) < 128) {
        pir = PIR_OFF;
      } else {
        pir = PIR_ON;
      }
      break;
      
    default:
      break;
  }
  
  return pir;
}
