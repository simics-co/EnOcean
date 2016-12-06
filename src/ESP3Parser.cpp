/*
  EnOcean ESP3 Parser

  This sketch gets the data of EnOcean radio telegram simply
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

#include "Arduino.h"
#include "SerialCommunication.h"
#include "EnOceanProfile.h"
#include "ESP3Parser.h"

static SerialCommunication SerialCom;
static uint16_t dataLength;
static uint8_t dataLength1;
static uint8_t dataLength2;
static uint8_t optLength;
static uint8_t packetType;
static uint8_t headerCrc8h;
static uint8_t header;
static uint8_t rorg;
static uint8_t exHeader;
static uint8_t exTelType;
static uint8_t senderId[4];
static uint8_t payload[4];
static uint8_t lengthPayload;
static uint8_t lengthOptData;
static uint8_t rssi;
static AfterReceivedTel pReceivedOpe;

static void reset();
static uint8_t getRORG();
static uint16_t getPacketLength();
static uint32_t getSenderId();
static uint32_t getPayload();
static void prettyPrint();

static uint8_t decodeSync(char aChar);
static uint8_t decodeDataLength1(char aChar);
static uint8_t decodeDataLength2(char aChar);
static uint8_t decodeOptLength(char aChar);
static uint8_t decodePacketType(char aChar);
static uint8_t decodeCrc8h(char aChar);
static uint8_t decodeHeader(char aChar);
static uint8_t decodeExHeader(char aChar);
static uint8_t decodeExTelType(char aChar);
static uint8_t decodeOrgId1(char aChar);
static uint8_t decodeOrgId2(char aChar);
static uint8_t decodeOrgId3(char aChar);
static uint8_t decodeOrgId4(char aChar);
static uint8_t decodeOrgId5(char aChar);
static uint8_t decodeOrgId6(char aChar);
static uint8_t decodeDstId1(char aChar);
static uint8_t decodeDstId2(char aChar);
static uint8_t decodeDstId3(char aChar);
static uint8_t decodeDstId4(char aChar);
static uint8_t decodePayload1(char aChar);
static uint8_t decodePayload2(char aChar);
static uint8_t decodePayload3(char aChar);
static uint8_t decodePayload4(char aChar);
static uint8_t decodeOptionalData(char aChar);
static uint8_t decodeCrc(char aChar);
static uint8_t decodeSubTelNum(char aChar);
static uint8_t decodeDbm(char aChar);
static uint8_t decodeCrc8d(char aChar);

enum STATE_ESP3 {
  STATE_SYNC = 0,
  STATE_DATA_LENGTH1,
  STATE_DATA_LENGTH2,
  STATE_OPT_LENGTH,
  STATE_PACKET_TYPE,
  STATE_CRC8H,
  STATE_HEADER,
  STATE_EX_HEADER,
  STATE_EX_TELTYPE,
  STATE_ORG_ID_1,
  STATE_ORG_ID_2,
  STATE_ORG_ID_3,
  STATE_ORG_ID_4,
  STATE_ORG_ID_5,
  STATE_ORG_ID_6,
  STATE_DST_ID_1,
  STATE_DST_ID_2,
  STATE_DST_ID_3,
  STATE_DST_ID_4,
  STATE_PAYLOAD_1,
  STATE_PAYLOAD_2,
  STATE_PAYLOAD_3,
  STATE_PAYLOAD_4,
  STATE_OPTIONAL_DATA,
  STATE_CRC,
  STATE_SUBTEL_NUM,
  STATE_DBM,
  STATE_CRC8D,
  STATE_ESP3_MAX
}; 

typedef uint8_t (*DecodeOpe)(char);

const DecodeOpe DecodeOpeSet[] = {
  decodeSync,           /* STATE_SYNC */
  decodeDataLength1,    /* STATE_DATA_LENGTH1 */
  decodeDataLength2,    /* STATE_DATA_LENGTH2 */
  decodeOptLength,      /* STATE_OPT_LENGTH */
  decodePacketType,     /* STATE_PACKET_TYPE */
  decodeCrc8h,          /* STATE_CRC8H */
  decodeHeader,         /* STATE_HEADER */
  decodeExHeader,       /* STATE_EX_HEADER */
  decodeExTelType,      /* STATE_EX_TELTYPE */
  decodeOrgId1,         /* STATE_ORG_ID_1 */
  decodeOrgId2,         /* STATE_ORG_ID_2 */
  decodeOrgId3,         /* STATE_ORG_ID_3 */
  decodeOrgId4,         /* STATE_ORG_ID_4 */
  decodeOrgId5,         /* STATE_ORG_ID_5 */
  decodeOrgId6,         /* STATE_ORG_ID_6 */
  decodeDstId1,         /* STATE_DST_ID_1 */
  decodeDstId2,         /* STATE_DST_ID_2 */
  decodeDstId3,         /* STATE_DST_ID_3 */
  decodeDstId4,         /* STATE_DST_ID_4 */
  decodePayload1,       /* STATE_PAYLOAD_1 */
  decodePayload2,       /* STATE_PAYLOAD_2 */
  decodePayload3,       /* STATE_PAYLOAD_3 */
  decodePayload4,       /* STATE_PAYLOAD_4 */
  decodeOptionalData,   /* STATE_OPTIONAL_DATA */
  decodeCrc,            /* STATE_CRC */
  decodeSubTelNum,      /* STATE_SUBTEL_NUM */
  decodeDbm,            /* STATE_DBM */
  decodeCrc8d           /* STATE_CRC8D */
};


ESP3Parser::ESP3Parser(AfterReceivedTel pAfterReceived)
{
  pReceivedOpe = pAfterReceived;
  reset();
}

void ESP3Parser::initialization()
{
  Serial.println("                                   ");
  Serial.println("                                   ");
  Serial.println("");
  Serial.println("    ID     R-ORG     Data       dBm");
  Serial.println("------------------------------------");
  
  SerialCom.Initialization();
  SerialCom.SetReceptOpe((ReceptionOpe*)DecodeOpeSet);
}

static void reset()
{
  exHeader = 0;
  exTelType = 0;
  senderId[0] = 0;
  senderId[1] = 0;
  senderId[2] = 0;
  senderId[3] = 0;
  payload[0] = 0;
  payload[1] = 0;
  payload[2] = 0;
  payload[3] = 0;
  rssi = 0;
}

static uint8_t getRORG() {
  return rorg;
}

static uint16_t getPacketLength() {
  return ((uint16_t(dataLength1) << 8) & 0xff00) + (dataLength2 & 0xff);
}

static uint32_t getSenderId() {
  uint32_t aResponse = ((uint32_t(senderId[0]) << 24) & 0xFF000000) + ((uint32_t(senderId[1])  << 16) & 0x00FF0000) + ((uint32_t(senderId[2]) << 8) & 0x0000FF00) + (uint32_t(senderId[3]) & 0x000000FF);
  return aResponse;
}

static uint32_t getPayload() {
  uint32_t aResponse = ((uint32_t(payload[0]) << 24) & 0xFF000000) + ((uint32_t(payload[1])  << 16) & 0x00FF0000) + ((uint32_t(payload[2]) << 8) & 0x0000FF00) + (uint32_t(payload[3]) & 0x000000FF);
  return aResponse;
}

static void prettyPrint()
{
  uint8_t  i;

  Serial.print(" ");
  for(i = 0; i < 4; i++) {
    if((senderId[i] & 0xF0) == 0x00) {
      Serial.print((senderId[i] & 0xF0), HEX);
    }
    Serial.print(senderId[i], HEX);
  }
  
  switch (rorg) {
    case 0: /* RPS Telegram */
      Serial.print("   RPS   ");
      if((payload[0] & 0xF0) == 0x00) {
        Serial.print((payload[0] & 0xF0), HEX);
      }
      Serial.print(payload[0], HEX);
      Serial.print("          ");
      break;
      
    case 1: /* 1BS Telegram */
      Serial.print("   1BS   ");
      if((payload[0] & 0xF0) == 0x00) {
        Serial.print((payload[0] & 0xF0), HEX);
      }
      Serial.print(payload[0], HEX);
      Serial.print("          ");
      break;
      
    case 2: /* 4BS Telegram */
      Serial.print("   4BS   ");
      for(i = 0; i < 4; i++) {
        if((payload[i] & 0xF0) == 0x00) {
          Serial.print((payload[i] & 0xF0), HEX);
        }
        Serial.print(payload[i], HEX);
        Serial.print(" ");
      }
      break;
      
    case 4: /* VLD Telegram */
      Serial.print("   VLD   ");
      for(i = 0; i < lengthPayload; i++) {
        if((payload[i] & 0xF0) == 0x00) {
          Serial.print((payload[i] & 0xF0), HEX);
        }
        Serial.print(payload[i], HEX);
        Serial.print(" ");
      }
      for(i = 0; i < (4 - lengthPayload); i++) {
        Serial.print("   ");
      }
      break;
  }
  
  Serial.print("  -");
  Serial.println(rssi , DEC);
}


/* STATE_SYNC */
static uint8_t decodeSync(char aChar)
{
  uint8_t state;
  if (aChar == START_BYTE) {
    state = STATE_DATA_LENGTH1;
  } else {
    state = STATE_SYNC;
  }
  return state;
}

/* STATE_DATA_LENGTH1 */
static uint8_t decodeDataLength1(char aChar)
{
  dataLength1 = aChar;
  return STATE_DATA_LENGTH2;
}

/* STATE_DATA_LENGTH2 */
static uint8_t decodeDataLength2(char aChar)
{
  dataLength2 = aChar;
  dataLength = getPacketLength();
  return STATE_OPT_LENGTH;
}

/* STATE_OPT_LENGTH */
static uint8_t decodeOptLength(char aChar)
{
  optLength = aChar;
  return STATE_PACKET_TYPE;
}

/* STATE_PACKET_TYPE */
static uint8_t decodePacketType(char aChar)
{
  packetType = aChar;
  return STATE_CRC8H;
}

/* STATE_CRC8H */
static uint8_t decodeCrc8h(char aChar)
{
  headerCrc8h = aChar;
  return STATE_HEADER;
}

/* STATE_HEADER */
static uint8_t decodeHeader(char aChar)
{
  uint8_t state;
  dataLength--;
  header = aChar;
  rorg = header & 0x0F;
  
  // extend header exit
  if ((header >> 4) & 0x01) {
    state = STATE_EX_HEADER;
    
    // no extend header
  } else if(rorg == 0x0F) {
    state = STATE_EX_TELTYPE;
    
  } else {
    switch((header >> 5) & 0x07) {
      case 0x00:
        state = STATE_ORG_ID_4;
        break;
      case 0x03:
        state = STATE_ORG_ID_1;
        break;
      default:
        state = STATE_ORG_ID_3;
        break;
    }
  }
  return state;
}

/* STATE_EX_HEADER */
static uint8_t decodeExHeader(char aChar)
{
  uint8_t state;
  dataLength--;
  exHeader = aChar;
  
  if(rorg == 0xFF) {
    state = STATE_EX_TELTYPE;
    
  } else {
    switch((header >> 5) & 0x07) {
      case 0x00:
        state = STATE_ORG_ID_1;
        break;
      case 0x03:
        state = STATE_ORG_ID_4;
        break;
      default:
        state = STATE_ORG_ID_3;
        break;
    }
  }
  return state;
}

/* STATE_EX_TELTYPE */
static uint8_t decodeExTelType(char aChar)
{
  uint8_t state;
  dataLength--;
  exTelType = aChar;
  
  switch((header >> 5) & 0x07) {
    case 0x00:
      state = STATE_ORG_ID_1;
      break;
    case 0x03:
      state = STATE_ORG_ID_4;
      break;
    default:
      state = STATE_ORG_ID_3;
      break;
  }
  return state;
}

/* STATE_ORG_ID_1 */
static uint8_t decodeOrgId1(char aChar)
{
  dataLength--;
  return STATE_ORG_ID_2;
}

/* STATE_ORG_ID_2 */
static uint8_t decodeOrgId2(char aChar)
{
  dataLength--;
  return STATE_ORG_ID_3;
}

/* STATE_ORG_ID_3 */
static uint8_t decodeOrgId3(char aChar)
{
  dataLength--;
  senderId[0] = aChar;
  return STATE_ORG_ID_4;
}

/* STATE_ORG_ID_4 */
static uint8_t decodeOrgId4(char aChar)
{
  dataLength--;
  senderId[1] = aChar;
  return STATE_ORG_ID_5;
}

/* STATE_ORG_ID_5 */
static uint8_t decodeOrgId5(char aChar)
{
  dataLength--;
  senderId[2] = aChar;
  return STATE_ORG_ID_6;
}

/* STATE_ORG_ID_6 */
static uint8_t decodeOrgId6(char aChar)
{
  uint8_t state;
  dataLength--;
  senderId[3] = aChar;
  
  if(((header >> 5) & 0x07) == 0x02) {
    state = STATE_DST_ID_1;
  } else {
    state = STATE_PAYLOAD_1;
  }
  return state;
}

/* STATE_DST_ID_1 */
static uint8_t decodeDstId1(char aChar)
{
  dataLength--;
  return STATE_DST_ID_2;
}

/* STATE_DST_ID_2 */
static uint8_t decodeDstId2(char aChar)
{
  dataLength--;
  return STATE_DST_ID_3;
}

/* STATE_DST_ID_3 */
static uint8_t decodeDstId3(char aChar)
{
  dataLength--;
  return STATE_DST_ID_4;
}

/* STATE_DST_ID_4 */
static uint8_t decodeDstId4(char aChar)
{
  dataLength--;
  return STATE_PAYLOAD_1;
}

/* STATE_PAYLOAD_1 */
static uint8_t decodePayload1(char aChar)
{
  uint8_t state;
  dataLength--;
  payload[0] = aChar;
  
  lengthOptData = exHeader & 0x0F;
  if((rorg == RORG_RPS) || (rorg == RORG_1BS) || ((rorg == RORG_VLD) && (dataLength == (lengthOptData + 1)))) {
    lengthPayload = 1;
    if(lengthOptData > 0) {
      state = STATE_OPTIONAL_DATA;
    } else {
      state = STATE_CRC;
    }
  } else {
    state = STATE_PAYLOAD_2;
  }
  return state;
}

/* STATE_PAYLOAD_2 */
static uint8_t decodePayload2(char aChar)
{
  uint8_t state;
  dataLength--;
  payload[1] = aChar;
  
  if((rorg == RORG_VLD) && (dataLength == (lengthOptData + 1))) {
    lengthPayload = 2;
    if(lengthOptData > 0) {
      state = STATE_OPTIONAL_DATA;
    } else {
      state = STATE_CRC;
    }
  } else {
    state = STATE_PAYLOAD_3;
  }
  return state;
}

/* STATE_PAYLOAD_3 */
static uint8_t decodePayload3(char aChar)
{
  uint8_t state;
  dataLength--;
  payload[2] = aChar;
  
  if((rorg == RORG_VLD) && (dataLength == (lengthOptData + 1))) {
    lengthPayload = 3;
    if(lengthOptData > 0) {
      state = STATE_OPTIONAL_DATA;
    } else {
      state = STATE_CRC;
    }
  } else {
    state = STATE_PAYLOAD_4;
  }
  return state;
}

/* STATE_PAYLOAD_4 */
static uint8_t decodePayload4(char aChar)
{
  uint8_t state;
  dataLength--;
  payload[3] = aChar;
  
  if((rorg == RORG_4BS) || ((rorg == RORG_VLD) && (dataLength == (lengthOptData + 1)))) {
    lengthPayload = 4;
    if(lengthOptData > 0) {
      state = STATE_OPTIONAL_DATA;
    } else {
      state = STATE_CRC;
    }
  } else {
    /* Ignore except for RPS, 1BS, 4BS, VLD(4bytes and less) */
    state = STATE_SYNC;
    reset();
  }
  return state;
}

/* STATE_OPTIONAL_DATA */
static uint8_t decodeOptionalData(char aChar)
{
  uint8_t state;
  
  lengthOptData--;
  if(lengthOptData > 0) {
    state = STATE_OPTIONAL_DATA;
  } else {
    state = STATE_CRC;
  }
  return state;
}

/* STATE_CRC */
static uint8_t decodeCrc(char aChar)
{
  return STATE_SUBTEL_NUM;
}

/* STATE_SUBTEL_NUM */
static uint8_t decodeSubTelNum(char aChar)
{
  return STATE_DBM;
}

/* STATE_DBM */
static uint8_t decodeDbm(char aChar)
{
  rssi = aChar;
  return STATE_CRC8D;
}

/* STATE_CRC8D */
static uint8_t decodeCrc8d(char aChar)
{
  uint32_t ID = getSenderId();
  uint32_t data = getPayload();
  
  if(pReceivedOpe != NULL) {
    (*pReceivedOpe)(rorg, ID, data, rssi);
  }

  if ((rorg == RORG_RPS) || (rorg == RORG_1BS) || (rorg == RORG_4BS) || (rorg == RORG_VLD)) { // RPS 1BS 4BS VLD
    prettyPrint();
  }
  
  reset();
  return STATE_SYNC;
}
