#ifndef _RELEWIFI_H_
#define _RELEWIFI_H_
#include "defines.h"
#include <vector>

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WebSocketsClient.h> 
#include <TimeLib.h>
#include <WiFiUdp.h>
//#include "dnsserver.h"
#include <ESP8266mDNS.h>
//#include <ArduinoOTA.h>
#include <ESP8266HTTPUpdateServer.h>
#include "EventHandler.h"
#include <IRrecv.h>
#include <IRsend.h>


class CIRWifi
{
  // variaveis globais pra auxiliar a programacao
  char strHeader[HEADER_SIZE + 1];
  char strSSID[SSID_SIZE+1];
  char strSSIDPassword[SSIDPASSWORD_SIZE + 1];
  std::vector<stExpressionItem *> Expression;
  std::vector<String> NearByWifi;
  // geolocalization
  String strPositionURL;
  char strPositionKey[GEO_POS_KEY_SIZE+1];
  bool bGeoKnown;              //posicao geografica conhecida
  double dbLatitude    = 0.0;
  double dbLongitude   = 0.0;
  // wheather key and URL
  char strWeatherKey[WEATHER_KEY_SIZE+1];
  String strWeatherURL;
  

  // MQTT
  char strBrokerURL[BROKERURL_SIZE + 1]; //URL do broker MQTT que se deseja utilizar
  char strBrokerPort[BROKERPORT_SIZE + 1]; // Porta do Broker MQTT
  char strDeviceName[DEVICENAME_SIZE + 1];
  bool bMqttOK;
  char strMqttUser[MQTTUSER_SIZE + 1];
  char strMqttPasscode[MQTTPASSCODE_SIZE + 1];
  byte byState;

  // SPEECH Alexa and Google Assistant
  char strSpeechKey[SPEECH_KEY_SIZE + 1];
  char strSpeechDevID[SPEECH_DEVICE_ID_SIZE + 1];
  bool bSpeechServerConnected;
  unsigned long ulHeartBeatTimestamp = 0;

  //IR Vars

  decode_results irDecodeRes;
  bool bIsReceivingIR;
  int iButtonIRIndex;
  stIRButton IRButtons[IR_TOTAL_BUTTONS];

  // timming vars
  WiFiUDP ntpUDP;
  ESP8266WebServer *p_Server;
  //DNSServer dnsServer;
  MDNSResponder mdns;
  char strTimeServer[TIMESERVER_SIZE + 1];
  unsigned long previousMillis = 0;
  unsigned long ulLastNTPTime;
  unsigned long ulHoursToNTPUpdate = 60000 * 5; // 5 min pra atualizar NTP
  const long lInterval = 10000;
  int16_t utc = -3;                        //UTC -3:00 Brazil

//resillient vars
  unsigned long ulPrevMillisRetry;
  byte byMaxStatePossible;

  ESP8266HTTPUpdateServer httpUpdater;
  
  public:
  bool bEnableExpressions;
  WiFiClient wifiClient; // Cria o objeto espClient
  PubSubClient MQTTClient;
  WebSocketsClient webSocket;
  CIRWifi();                             // constructor
  ~CIRWifi();                           // destuctor
  void Format();                           // format eeprom
  void Init();
  bool InitAccessPointMode(void);        // initializes ACCESS POINT 
  bool InitStationMode();
  bool InitMQTT();
  bool Connect_Wifi_APMode();
  bool Connect_Wifi_STAMode();
  bool Connect_MQTT();
  bool InitWebServer();
  bool EndWebServer();
  void LoadSettings();
  void SaveSettings();
  void Reset();
  void SaveGeoPositioning();
  void LoadExpressions(int iInitMemoryParam);
  void SaveExpressions(int iInitMemoryParam);
  void SaveNearbySSIDs();
  void SaveIRButtons();
  bool AddEventHandler(eEventHandler enEvtParam,String strPayLoadParam);
  bool RemoveSensorAtExpression(int iIndexExpressionParam,int iIndexSensorParam);
  void Update();
  bool ChangeState(byte byNewState);
  void WelcomePage();
  void ClearPage();
  void SubmitPage();
  void BtnTVOnOffPage();
  bool GetRelayState(int iRelayParam);
  bool UpdateNTPTime(bool bForceUpdateNowParam);
  bool IsDayLightSaving();
  String GetExpressions();
  //void ToggleExpressions(bool bEnableParam);
  void SetUserPosition(String strNameParam,double dbLatitudeParam,double dbLongitudeParam);
  void ChangeEffectOperatorExpression(int iExpressionIndexParam,char chEffectParam);
  void ChangeCauseOperatorExpression(int iExpressionIndexParam,int iIndexParam, char chOperatorParam);
  bool IsGeoKnown() {return bGeoKnown;};
  double GetLatitudePosition() { return dbLatitude;};
  double GetLongitudePosition() { return dbLongitude;};
  String GetWeatherAPIURL() { return strWeatherURL;};
  String GetWeatherAPIKey() { return strWeatherKey;};
  String GetPositionURLByIndex(int iIndexParam);
  String GetWeatherURLByIndex(int iIndexParam);
  int GetIndexByURLPosition();
  int GetIndexByURLWeather();
  bool SetWeatherURLByIndex(int iIndexParam);
  bool SetPositionURLByIndex(int iIndexParam);
  void InitSpeech();
  void HandleSpeechEvent(WStype_t type, uint8_t * payload, size_t length);
  void InformRelayStateOnSpeechServer(String deviceId, String value);
  void SendIRCommand(int iIRCommandParam);
};


#endif
