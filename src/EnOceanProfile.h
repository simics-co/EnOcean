#ifndef EnOceanProfile_h
#define EnOceanProfile_h

#define EEP_1BS_LRN_BIT 0x08000000
#define EEP_4BS_LRN_BIT 0x00000008

enum EEP_TYPE {
  EEP_F6_02_04 = 0, /* Rocker Switch */
  EEP_D5_00_01,     /* Contact Sensor */
  EEP_A5_02_05,     /* Temperature Sensor */
  EEP_A5_02_30,     /* Temperature Sensor */
  EEP_A5_07_01,     /* Occupancy Sensor */
  EEP_A5_08_02,     /* Occupancy Sensor */
  EEP_TYPE_MAX
}; 

class EnOceanProfile
{
public:
  uint8_t getSwitchStatus(EEP_TYPE type, uint32_t dataRPS);
  uint8_t getContact(EEP_TYPE type, uint32_t data1BS);
  float getTemperature(EEP_TYPE type, uint32_t data4BS);
  uint8_t getPIRStatus(EEP_TYPE type, uint32_t data4BS);

private:
};

#endif // EnOceanProfile_h
