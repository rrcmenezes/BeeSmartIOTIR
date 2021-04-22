#ifndef _DEFINES_H_
#define _DEFINES_H_
#include <Arduino.h>
#include <EEPROM.h>
#include <vector>
#include "EventHandler.h"




#define DOMINGO   (byte)64
#define SEGUNDA   (byte)32
#define TERCA     (byte)16
#define QUARTA     (byte)8
#define QUINTA     (byte)4
#define SEXTA      (byte)2
#define SABADO     (byte)1

#define VERSION "ACR3.6"

#define MEM_SIZE                    4096
#define HEADER_INIT                 0
#define HEADER_SIZE                 10
#define SSID_MEM_INIT               HEADER_INIT + HEADER_SIZE   // inicio na memoria eeeprom SSID
#define SSID_SIZE                   32
#define SSIDPASSWORD_MEM_INIT       SSID_MEM_INIT + SSID_SIZE  // inicio na memoria eeeprom SSID
#define SSIDPASSWORD_SIZE           32   
#define DEVICENAME_MEM_INIT         SSIDPASSWORD_MEM_INIT + SSIDPASSWORD_SIZE   // inicio na memoria eeeprom SSID
#define DEVICENAME_SIZE             32 
#define MQTTUSER_MEM_INIT           DEVICENAME_MEM_INIT +  DEVICENAME_SIZE // inicio na memoria eeeprom SSID
#define MQTTUSER_SIZE               16
#define MQTTPASSCODE_MEM_INIT       MQTTUSER_MEM_INIT + MQTTUSER_SIZE // inicio na memoria eeeprom SSID
#define MQTTPASSCODE_SIZE           32
#define BROKERURL_MEM_INIT          MQTTPASSCODE_MEM_INIT + MQTTPASSCODE_SIZE // inicio na memoria eeeprom SSID
#define BROKERURL_SIZE              60  
#define BROKERPORT_MEM_INIT         BROKERURL_MEM_INIT + BROKERURL_SIZE  // inicio na memoria eeeprom SSID
#define BROKERPORT_SIZE             sizeof(unsigned int)  
#define TIMESERVER_MEM_INIT         BROKERPORT_MEM_INIT + BROKERPORT_SIZE  // inicio na memoria eeeprom SSID
#define TIMESERVER_SIZE             32
#define GEO_POS_URL_MEM_INIT        TIMESERVER_MEM_INIT + TIMESERVER_SIZE // inicio na memoria eeeprom SSID
#define GEO_POS_URL_SIZE            sizeof(int)
#define GEO_POS_KEY_MEM_INIT        GEO_POS_URL_MEM_INIT + GEO_POS_URL_SIZE // inicio na memoria eeeprom SSID
#define GEO_POS_KEY_SIZE            52
#define GEO_LAT_MEM_INIT            GEO_POS_KEY_MEM_INIT + GEO_POS_KEY_SIZE
#define GEO_LAT_SIZE                sizeof(double)
#define GEO_LONG_MEM_INIT           GEO_LAT_MEM_INIT + GEO_LAT_SIZE
#define GEO_LONG_SIZE               sizeof(double)
#define WEATHER_URL_MEM_INIT        GEO_LONG_MEM_INIT + GEO_LONG_SIZE
#define WEATHER_URL_SIZE            sizeof(int)
#define WEATHER_KEY_MEM_INIT        WEATHER_URL_MEM_INIT + WEATHER_URL_SIZE
#define WEATHER_KEY_SIZE            32
#define SPEECH_KEY_MEM_INIT         WEATHER_KEY_MEM_INIT + WEATHER_KEY_SIZE  // inicio na memoria eeprom
#define SPEECH_KEY_SIZE             38
#define SPEECH_DEVICE_ID_MEM_INIT   SPEECH_KEY_MEM_INIT + SPEECH_KEY_SIZE
#define SPEECH_DEVICE_ID_SIZE       25
#define NEARBY_SSID_QUANT_MEM_INIT  SPEECH_DEVICE_ID_MEM_INIT + SPEECH_DEVICE_ID_SIZE  // inicio na memoria eeprom
#define NEARBY_SSID_QUANT_SIZE      sizeof(byte)
#define NEARBY_SSID_MEM_INIT        NEARBY_SSID_QUANT_MEM_INIT + NEARBY_SSID_QUANT_SIZE
#define NEARBY_SSID_REG_SIZE        32
#define NEARBY_SSID_MAX_REGS        15                                     // numero total de registros permitidos

#define IR_TV_ONOFF_FREQ_MEM_INIT   NEARBY_SSID_MEM_INIT + ( NEARBY_SSID_REG_SIZE * NEARBY_SSID_MAX_REGS)
#define IR_TV_ONOFF_FREQ_SIZE       sizeof(uint16_t)
#define IR_TV_ONOFF_SIZE_MEM_INIT   IR_TV_ONOFF_FREQ_MEM_INIT + IR_TV_ONOFF_FREQ_SIZE
#define IR_TV_ONOFF_SIZE_SIZE       sizeof(uint16_t)
#define IR_TV_ONOFF_RAW_MEM_INIT    IR_TV_ONOFF_SIZE_MEM_INIT + IR_TV_ONOFF_SIZE_SIZE
#define IR_TV_ONOFF_RAW_SIZE        sizeof(uint16_t) * 100

#define IR_TV_INPUT_FREQ_MEM_INIT   IR_TV_ONOFF_RAW_MEM_INIT + IR_TV_ONOFF_RAW_SIZE
#define IR_TV_INPUT_FREQ_SIZE       sizeof(uint16_t)
#define IR_TV_INPUT_SIZE_MEM_INIT   IR_TV_INPUT_FREQ_MEM_INIT + IR_TV_INPUT_FREQ_SIZE
#define IR_TV_INPUT_SIZE_SIZE       sizeof(uint16_t)
#define IR_TV_INPUT_RAW_MEM_INIT    IR_TV_INPUT_SIZE_MEM_INIT + IR_TV_INPUT_SIZE_SIZE
#define IR_TV_INPUT_RAW_SIZE        sizeof(uint16_t) * 100

#define IR_TV_VOLUP_FREQ_MEM_INIT   IR_TV_INPUT_RAW_MEM_INIT + IR_TV_INPUT_RAW_SIZE
#define IR_TV_VOLUP_FREQ_SIZE       sizeof(uint16_t)
#define IR_TV_VOLUP_SIZE_MEM_INIT   IR_TV_VOLUP_FREQ_MEM_INIT + IR_TV_VOLUP_FREQ_SIZE
#define IR_TV_VOLUP_SIZE_SIZE       sizeof(uint16_t)
#define IR_TV_VOLUP_RAW_MEM_INIT    IR_TV_VOLUP_SIZE_MEM_INIT + IR_TV_VOLUP_SIZE_SIZE
#define IR_TV_VOLUP_RAW_SIZE        sizeof(uint16_t) * 100
#define MEM_INIT_EXPRESSION_POINTER   IR_TV_VOLUP_RAW_MEM_INIT + IR_TV_VOLUP_RAW_SIZE




#define DNS_PORT  53
#define INTERVAL_RETRY  120000
#define GEO_MATCHES_RATIO 0.5    // se acertar mais 50% das redes wifi ao lado, ta de boa
#define HEARTBEAT_INTERVAL 300000 // 5 Minutes to informm speech sinric system i am alive



typedef struct Vector2D
{
  float fLat;
  float fLong;
};

typedef struct ClientPosition
{
  String strName;
  Vector2D vPos; 
};


enum eModuleState
{
  NONE = 1,
  AP_WEBSERVER,
  STA_WEBSERVER  
};

enum eIRButtons
{
  IR_TV_ONOFF = 0,
  IR_TV_INPUT,
  IR_TV_VOLUP,
  IR_TV_VOLDOWN,
  IR_PAYTV_ONOFF,
  IR_PAYTV_INPUT,
  IR_PAYTV_VOLUP,
  IR_PAYTV_VOLDOWN,
  IR_TOTAL_BUTTONS  
};

typedef struct stIRButton
{
  uint16_t uiFreq;
  uint16_t uiSize;
  uint16_t *p_uiRawData;
};



typedef enum eEventHandler
{
  EVT_PRD_HANDLER = 1,
  EVT_GPS_HANDLER,
  EVT_WTR_HANDLER
};

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    int i;
    for (i = 0; i < sizeof(value); i++)
        EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    int i;
    for (i = 0; i < sizeof(value); i++)
        *p++ = EEPROM.read(ee++);
    return i;
}

typedef struct stExpressionItem
{
  std::vector<CEventHandler *> Sensors;
  std::vector<char> CauseOperators;  // possiveis operadores: AND, OR, XOR
  char strEffectOperator[4];
};

// ==================== start of TUNEABLE PARAMETERS ====================
// An IR detector/demodulator is connected to GPIO pin 14
// e.g. D6 on a NodeMCU board.
const uint16_t kRecvPin = 12;
// e.g. D2 on a NodeMCU board.
const uint16_t kSendPin = 4;

// The Serial connection baud rate.
// i.e. Status message will be sent to the PC at this baud rate.
// Try to avoid slow speeds like 9600, as you will miss messages and
// cause other problems. 115200 (or faster) is recommended.
// NOTE: Make sure you set your Serial Monitor to the same speed.
const uint32_t kBaudRate = 115200;

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
const uint16_t kCaptureBufferSize = 1024;

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// This parameter is an interesting trade-off. The longer the timeout, the more
// complex a message it can capture. e.g. Some device protocols will send
// multiple message packets in quick succession, like Air Conditioner remotes.
// Air Coniditioner protocols often have a considerable gap (20-40+ms) between
// packets.
// The downside of a large timeout value is a lot of less complex protocols
// send multiple messages when the remote's button is held down. The gap between
// them is often also around 20+ms. This can result in the raw data be 2-3+
// times larger than needed as it has captured 2-3+ messages in a single
// capture. Setting a low timeout value can resolve this.
// So, choosing the best kTimeout value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed kMaxTimeoutMs. Typically 130ms.
#if DECODE_AC
// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t kTimeout = 50;
#else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t kTimeout = 15;
#endif  // DECODE_AC
// Alternatives:
// const uint8_t kTimeout = 90;
// Suits messages with big gaps like XMP-1 & some aircon units, but can
// accidentally swallow repeated messages in the rawData[] output.
//
// const uint8_t kTimeout = kMaxTimeoutMs;
// This will set it to our currently allowed maximum.
// Values this high are problematic because it is roughly the typical boundary
// where most messages repeat.
// e.g. It will stop decoding a message and start sending it to serial at
//      precisely the time when the next message is likely to be transmitted,
//      and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the kTimeout value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
const uint16_t kMinUnknownSize = 12;
// ==================== end of TUNEABLE PARAMETERS ====================
         

#endif
