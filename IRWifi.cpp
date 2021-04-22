#include "IRWifi.h"
#include "EventGPSHandler.h"
#include "EventWeatherHandler.h"
#include "EventPeriodHandler.h" 


#include <EEPROM.h>
#include <ArduinoJson.h>
#include <StreamString.h>
#include <IRutils.h>

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
// D8 is GPIO15
IRsend irsend(kSendPin); // Set the GPIO to be used to sending the message.

const char HeaderPage[] PROGMEM = "<!DOCTYPE html>\n<html lang=\\\"pt-BR\\\">\n<head>\n<meta charset=\\\"UTF-8\\\">     \n<style>\ndiv.container { width: 100%; border: 1px solid gray;}\nheader, footer {padding: 1em; color: white; background-color: green; clear: left; text-align: center;}\n\"article {margin-left: 17px; padding: 1em; text-align: justify;}\n</style>\n</head>\n<body>\n<div class=\\\"container\\\">\n<header> <h1>Automação Residencial</h1> </header>\n<article>\n<h1>Módulo Relé</h1>\n<p>Bem vindo ao BeeSmartIOT. Você deve definir o nome do dispositivo, endereço do servidor e porta MQTT, sua rede WIFI e senha para que o dispositivo se comunique. Uma vez conectado, você poderá ter acesso a diversos servicos disponíveis para automação.</p>\n<form method='get' action='setting'>\n<p align='center'>";
const char FooterPage[] PROGMEM = "</p>\n</form>\n</article>\n<footer>Copyright &copy; BeeSmartIOT ver. {{VERSION}}\n</footer>\n</div>\n</body>\n</html>";
/***********************************************************************************************************/
CIRWifi::CIRWifi()
{
  strSSID[0] = '\0';
  strSSIDPassword[0] = '\0';
  strMqttUser[0]='\0';
  p_Server = NULL;
  byState = NONE;
  MQTTClient.setClient(wifiClient);
  //digitalWrite(12, HIGH);
  //digitalWrite(13, HIGH);
  bEnableExpressions = true;
  strTimeServer[0] = '\0';
  ulLastNTPTime = 0;
  strWeatherKey[0] ='\0';
  strPositionKey[0] ='\0';
  strSpeechKey[0] ='\0';
  strSpeechDevID[0] ='\0';
  bSpeechServerConnected = false;
  byMaxStatePossible = NONE;
  //strEffectOperator = "TRUE_ENABLE_ELSE"; // TRUE_ENABLE/TRUE_DISABLE/TRUE_ENABLE_ELSE/TRUE_DISABLE_ELSE/TRUE_SENSOR
  bIsReceivingIR = false;
  for (int i = 0; i < IR_TOTAL_BUTTONS; i++)
  {
    IRButtons[i].uiSize = 0;
    IRButtons[i].uiFreq = 0; 
    IRButtons[i].p_uiRawData = NULL;
  }
}

/***********************************************************************************************************/
CIRWifi::~CIRWifi()
{
  if (p_Server)
  {
    delete p_Server;
    p_Server = NULL;
  }
}

/***********************************************************************************************************/
void CIRWifi::Init()
{
  EEPROM.begin(MEM_SIZE);

  
  //ArduinoOTA.onStart([]() {
  //  Serial.println(F("OTA Start\n"));
  //});
  //ArduinoOTA.onEnd([]() {
  //  DEBUG_PRINTLN("\nEnd");
  //});
  //ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  //  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  //});
  //ArduinoOTA.onError([](ota_error_t error) {
  //  Serial.printf("Error[%u]: ", error);
  //  if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
  //  else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
   // else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
  //  else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
  //  else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
 // });
  
  
  
  LoadSettings();  // load from eeprom
 
  //ArduinoOTA.setHostname(strDeviceName);
  // No authentication by default 
  //ArduinoOTA.setPassword((const char *)"123");
  //ArduinoOTA.begin();

  
    
  if ( strcmp(strSSID,"")) 
  {
    byMaxStatePossible = STA_WEBSERVER;
    if (!ChangeState(STA_WEBSERVER))
    {
      ChangeState(AP_WEBSERVER);
    }
  }
  else
  {
    byMaxStatePossible = AP_WEBSERVER;
    ChangeState(AP_WEBSERVER);
  }
  
  
  if (UpdateNTPTime(true))
  {
    randomSeed(now());
  }
  irsend.begin(); //INICIALIZA A FUNÇÃO
}

/**************************************************************************************************************************************************************************************************************************************/



/**************************************************************************************************************************************************************************************************************************************/
void CIRWifi::Format()
{
  int iHeaderStringSize;
  String strHeader;
  strHeader = VERSION;

  iHeaderStringSize = strHeader.length();
  for (int i = 0; i < iHeaderStringSize; i++)
  {
    EEPROM.write(HEADER_INIT + i, strHeader[i]);
  }
  
  for (int i = iHeaderStringSize; i < HEADER_SIZE; i++)
  {
    EEPROM.write(HEADER_INIT + i, '\0');
  }

  for (int i = HEADER_INIT + HEADER_SIZE ; i < MEM_SIZE ; i++) {
    EEPROM.write(i, 0);
  }

  EEPROM.commit();
  
}

/**************************************************************************************************************************************************************************************************************************************/
void CIRWifi::Update()
{
  
  if (!p_Server)
  {
    InitWebServer();      
  }
  else
  {
    p_Server->handleClient();
  }
  //ArduinoOTA.handle();

  UpdateNTPTime(false);
  
  switch (byState) 
  {
    case AP_WEBSERVER:
     {
      //dnsServer.processNextRequest();
      unsigned long currMillisRetry = millis();
      if ((currMillisRetry - ulPrevMillisRetry) >= INTERVAL_RETRY && (byMaxStatePossible > AP_WEBSERVER))  // trying upgrade status  
      { 
        ulPrevMillisRetry = currMillisRetry;
        Serial.println(F("MaxStatePossible > AP_WEBSERVER, trying to upgrade status to STA_WEBSERVER"));
        if (!ChangeState(STA_WEBSERVER)) // nao conseguiu entrar no wifi, reestabelece o AP_MODE
        {
          InitAccessPointMode();
        }
      }
      break;
     }      
    case STA_WEBSERVER:
     {
      //dnsServer.processNextRequest();
      if (!Connect_Wifi_STAMode())
      {
        ChangeState(AP_WEBSERVER);
      }
      else
      {
        unsigned long currMillis = millis();
        
        webSocket.loop();        
        if(bSpeechServerConnected)
        {
      
          // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night.
          if((currMillis - ulHeartBeatTimestamp) > HEARTBEAT_INTERVAL) 
          {
            ulHeartBeatTimestamp = currMillis;
            webSocket.sendTXT("H");
          }
        }
        
        if (((currMillis - ulPrevMillisRetry) >= INTERVAL_RETRY) && strcmp(strBrokerURL,"") && strcmp(strBrokerPort,""))  // trying upgrade status 
        {
          ulPrevMillisRetry = currMillis;        
          bMqttOK = Connect_MQTT();
        }
      }
      if (bMqttOK)
      {
         MQTTClient.loop();
      }
      break;
     }
  }
  
 bool bResult;
 bool bTurnOn;
 
 if (Expression.size() && bEnableExpressions)
 {
   for (int i = 0 ; i < Expression.size(); i++)
   {
    if (Expression[i]->CauseOperators.size() == 0 && Expression[i]->Sensors.size() > 0)
    {
      Expression[i]->Sensors[0]->Update();
      bResult = Expression[i]->Sensors[0]->ConditionsOK();
    }
    else
    {
      // too many sensors 
      for (int j = 0; j < Expression[i]->CauseOperators.size(); j++)
      {
        if (j == 0)
        {
          Expression[i]->Sensors[0]->Update();
          Expression[i]->Sensors[1]->Update();
          if (Expression[i]->CauseOperators[j] == 'A') bResult = Expression[i]->Sensors[0]->ConditionsOK() && Expression[i]->Sensors[1]->ConditionsOK();
          if (Expression[i]->CauseOperators[j] == 'O')  bResult = Expression[i]->Sensors[0]->ConditionsOK() || Expression[i]->Sensors[1]->ConditionsOK();
        }
        else
        {
          Expression[i]->Sensors[j+1]->Update();
          if (Expression[i]->CauseOperators[j] == 'A') bResult = bResult && Expression[i]->Sensors[j+1]->ConditionsOK();
          if (Expression[i]->CauseOperators[j] == 'O')  bResult = bResult || Expression[i]->Sensors[j+1]->ConditionsOK();
        }
      }    
    }
    /*
    if (!strcmp(Expression[i]->strEffectOperator,"TEB")) // true enable both relays
    {
      if (bResult)
      {
        bTurnOn = true;
        iRelayAffected = 0;
      }
    }
    if (!strcmp(Expression[i]->strEffectOperator,"TDB")) // true disable both relays
    {
      if (bResult)
      {
        bTurnOn = false;
        iRelayAffected = 0;
      }
    }
    if (!strcmp(Expression[i]->strEffectOperator,"TED")) // ture enable else disable both relays
    {
      if (bResult)
      {
        bTurnOn = true;
      }
      else
      {
        bTurnOn = false;
      }
      iRelayAffected = 0;
    }
    if (!strcmp(Expression[i]->strEffectOperator,"TDE")) // true disable else disable both relays
    {
      if (bResult)
      {
        bTurnOn = false;
      }
      else
      {
        bTurnOn = true;
      }
      iRelayAffected = 0;
    }
    if (!strcmp(Expression[i]->strEffectOperator,"TR")) // true randomize
    {
      if (bResult)
      {
        // randomiza aqui (TO DO)
      }  
    } 
   }

   if (iRelayAffected != -1)
   {
     if (bTurnOn)
     {
      RelayOn(iRelayAffected);
      InformRelayStateOnSpeechServer(strSpeechDevID,"ON");
     }
     else
     {
      RelayOff(iRelayAffected);
      InformRelayStateOnSpeechServer(strSpeechDevID,"OFF");
     }
   }
   */
  }
 }

 if (bIsReceivingIR)
 {
  if (irrecv.decode(&irDecodeRes))
  {
    if (irDecodeRes.decode_type != UNKNOWN)
    {
      bIsReceivingIR = false;
      irrecv.disableIRIn();  // disable the receiver
      Serial.print(resultToHumanReadableBasic(&irDecodeRes));
      yield();  // Feed the WDT as the text output can take a while to print.
      // Output RAW timing info of the result.
      Serial.println(resultToTimingInfo(&irDecodeRes));
      yield();  // Feed the WDT (again)
      // Output the results as source code
      Serial.println(resultToSourceCode(&irDecodeRes));

      if (IRButtons[iButtonIRIndex].p_uiRawData)
      {
        delete IRButtons[iButtonIRIndex].p_uiRawData;
        IRButtons[iButtonIRIndex].p_uiRawData = NULL;
      }
      IRButtons[iButtonIRIndex].uiSize = getCorrectedRawLength(&irDecodeRes);
      IRButtons[iButtonIRIndex].p_uiRawData = new uint16_t[ IRButtons[iButtonIRIndex].uiSize];     
      IRButtons[iButtonIRIndex].uiFreq = irDecodeRes.bits;
      //memcpy(IRButtons[iButtonIRIndex].p_uiRawData,(uint16_t *)irDecodeRes.rawbuf,sizeof(uint16_t) * IRButtons[iButtonIRIndex].uiSize);
        // Dump data
      for (uint16_t i = 1; i < irDecodeRes.rawlen; i++) 
      {
        uint32_t usecs;
        for (usecs = irDecodeRes.rawbuf[i] * kRawTick; usecs > UINT16_MAX; usecs -= UINT16_MAX)
        {
          Serial.printf("MERDA");
          //IRButtons[iButtonIRIndex].p_uiRawData[i] = 
        }
       IRButtons[iButtonIRIndex].p_uiRawData[i-1] = usecs;
      }
      SaveIRButtons();
    }
  }
 }
 
 
    unsigned long currentMillis = millis();



    
    if ((currentMillis - previousMillis) >= lInterval) 
    {
      byte byDay = 1 << (7 - (byte)weekday());
      Serial.println(F("---------"));
      Serial.println("State: " + String(byState) + F(" -- (1) NONE  (2) AP_WEBSERVER  (3) STA_WEBSERVER"));
 //     DEBUG_PRINTLN("MaxStatePossible: " + String(byMaxStatePossible));
 //     DEBUG_PRINTLN("variavel byDay " + String(byDay));
      Serial.print(F("Lat="));
      Serial.println(dbLatitude,6);
      Serial.print(F("Long="));
      Serial.println(dbLongitude,6);

      Serial.print(F("Year:"));
      Serial.print(String(year()));
      Serial.print(F(" Month:"));
      Serial.print(String(month()));
      Serial.print(F(" Day:"));
      Serial.print(String(day()));
      Serial.print(F( "Hour:"));
      Serial.print(String(hour()));
      Serial.print(F(" Min:"));
      Serial.println(String(minute()));
 //     DEBUG_PRINTLN(" Secs: " + String(second()));

      //Serial.println(String(now()));
      
      Serial.println(GetExpressions());
      
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      //if (Connect_Wifi())
      //{
      //  Connect_MQTT();
    }
    
}
/*********************************************************************************************************************************/
String CIRWifi::GetExpressions()
{
  unsigned int i;
  unsigned int j;  
  String strExp;

  strExp = "";

  if (bEnableExpressions)
  {
    for (i = 0 ; i < Expression.size(); i++)
    {
      strExp += "Expression" + String(i) + ": se ";
      for (j = 0; j < Expression[i]->Sensors.size(); j++)
      {
        strExp+= Expression[i]->Sensors[j]->PrintExpression();
        if (j < Expression[i]->CauseOperators.size())
        {
          if (Expression[i]->CauseOperators[j] == 'A') strExp+= F(" E ");
          if (Expression[i]->CauseOperators[j] == 'O') strExp+= F(" OU ");
          if (Expression[i]->CauseOperators[j] == 'X') strExp+= F(" OU EXCLUSIVO ");
        } 
      }
      if (!strcmp(Expression[i]->strEffectOperator,"TE")) strExp+= F(" -> LIGA\n");
      if (!strcmp(Expression[i]->strEffectOperator,"TD")) strExp+= F(" -> DESLIGA\n");
      if (!strcmp(Expression[i]->strEffectOperator,"TED")) strExp+= F(" -> LIGA / DESLIGA\n");
      if (!strcmp(Expression[i]->strEffectOperator,"TDE")) strExp+= F(" -> DESLIGA / LIGA\n");
      if (!strcmp(Expression[i]->strEffectOperator,"TR")) strExp+= F(" -> ALEATORIO\n");    
    }
  }
  else
  {
    strExp = F("Exmpressoes desligadas");
  }
  return strExp;
}
/*********************************************************************************************************************************/
/*
void CIRWifi::ToggleExpressions(bool bEnableParam)
{
  bEnableExpressions = bEnableParam;
}
*/

/*********************************************************************************************************************************/
void CIRWifi::ChangeCauseOperatorExpression(int iExpressionIndexParam,int iIndexParam, char chOperatorParam)
{
  if (iExpressionIndexParam < Expression.size())
  {
    if (iIndexParam < Expression[iExpressionIndexParam]->CauseOperators.size())
    {
      if (toupper(chOperatorParam) == 'A' || toupper(chOperatorParam == 'O') || toupper(chOperatorParam == 'X'))
      {
        Expression[iExpressionIndexParam]->CauseOperators[iIndexParam] = chOperatorParam;
        SaveExpressions(MEM_INIT_EXPRESSION_POINTER);
      }
    }  
  }
}
/*********************************************************************************************************************************/
void CIRWifi::ChangeEffectOperatorExpression(int iExpressionIndexParam,char chEffectParam)
{
  bool bChanged = false;
  
  if (iExpressionIndexParam < Expression.size())
  {   
      if (chEffectParam == '1')
      {
        strcpy(Expression[iExpressionIndexParam]->strEffectOperator,"TE");
        bChanged = true;
      }
      if (chEffectParam == '2')
      {
        strcpy(Expression[iExpressionIndexParam]->strEffectOperator,"TD");
        bChanged = true;
      }
      if (chEffectParam == '3')
      {
        strcpy(Expression[iExpressionIndexParam]->strEffectOperator,"TED");
        bChanged = true;
      }
      if (chEffectParam == '4')
      {
        strcpy(Expression[iExpressionIndexParam]->strEffectOperator,"TDE");
        bChanged = true;
      }
  }
  if (bChanged)
  {
    SaveExpressions(MEM_INIT_EXPRESSION_POINTER);
  }
  
}
/*********************************************************************************************************************************/
bool CIRWifi::UpdateNTPTime(bool bForceUpdateNowParam)
{
  const int NTP_PACKET_SIZE= 48;    // NTP time stamp is in the first 48 bytes of the message
  byte packetBuffer[ NTP_PACKET_SIZE];   // buffer to hold incoming and outgoing packets 
  
  if (WiFi.status() == WL_CONNECTED && strcmp(strTimeServer,""))
  { 
    if (((millis() - ulLastNTPTime) >= ulHoursToNTPUpdate) || bForceUpdateNowParam)
    {
      Serial.println(F("Updating NTP"));
      ulLastNTPTime = millis();
      //DEBUG_PRINTLN("as 3 variaveis nao inicializadas");
      //DEBUG_PRINTLN(now());
      //DEBUG_PRINTLN(tLastNTPTime);
      //DEBUG_PRINTLN(ulHoursToNTPUpdate);
      
      ulHoursToNTPUpdate = 60000 * 5; // 5 minutes to update if anything goes wrong
      
      int udpInited = ntpUDP.begin(8888); // open socket on arbitrary port
    

       // set all bytes in the buffer to 0
       memset(packetBuffer, 0, NTP_PACKET_SIZE); 

       // Initialize values needed to form NTP request
      // (see URL above for details on the packets)
      packetBuffer[0] = 0b11100011;   // LI, Version, Mode
      packetBuffer[1] = 0;    // Stratum, or type of clock
      packetBuffer[2] = 6;    // Polling Interval
      packetBuffer[3] = 0xEC;  // Peer Clock Precision
     
      // 8 bytes of zero for Root Delay & Root Dispersion
      packetBuffer[12] = 49; 
      packetBuffer[13] = 0x4E;
      packetBuffer[14] = 49;
      packetBuffer[15] = 52;
        
      // Fail if WiFiUdp.begin() could not init a socket
      if (! udpInited)
      {     
        return false;
      }
    
      // Clear received data from possible stray received packets
      //ntpUDP.flush();

      ntpUDP.beginPacket(strTimeServer, 123); //NTP requests are to port 123
      ntpUDP.write(packetBuffer,NTP_PACKET_SIZE);
      ntpUDP.endPacket(); 

      

      unsigned long ulMillisInitial = millis();
      unsigned long ulSecondsToWait = 10 * 1000;

      while (ntpUDP.parsePacket() == 0)
      {
        if (millis() - ulMillisInitial > ulSecondsToWait)
        {
          return false;
        }
      }
  

      // Read and discard the first useless bytes
      // Set useless to 32 for speed; set to 40 for accuracy.
      const byte useless = 40;
      for (int i = 0; i < useless; ++i)
      {
        ntpUDP.read();
      }
    
      // Read the integer part of sending time
      unsigned long ulTime = ntpUDP.read();  // NTP time
      for (int i = 1; i < 4; i++)
      {
        ulTime = ulTime << 8 | ntpUDP.read();
      }
    
      // Round to the nearest second if we want accuracy
      // The fractionary part is the next byte divided by 256: if it is
      // greater than 500ms we round to the next second; we also account
      // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
      // additionally, we account for how much we delayed reading the packet
      // since its arrival, which we assume on average to be pollIntv/2.
      //ulTime += (ntpUDP.read() > 115 - pollIntv/8);
    
      // Discard the rest of the packet
      ntpUDP.flush();
  
      // convert NTP time to unix time (since 1st Jan 1970)
      ulTime -= 2208988800ul;
      // TimeZone adjust
      ulTime = ulTime + (3600 * utc);
  
      
      setTime(ulTime); 

      if (IsDayLightSaving())
      {
        Serial.print(F("DLSav YES"));
        setTime(ulTime + 3600); // adiciona 3600 segundos (1 hora)  se for horario de verao
      }
      else
      {
        Serial.print(F("DLSav NO"));
      }
            
      ulLastNTPTime = millis();
     
  
      ulHoursToNTPUpdate = 3600000 * 10;  // next update 10 hours after

      //DEBUG_PRINTLN("Time Successfully update to:");
     // DEBUG_PRINT("Year: "+ String(year()));
     // DEBUG_PRINT(" Month: " + String(month()));
     // DEBUG_PRINT(" Day: " + String(day()));
     // DEBUG_PRINT(" Hour: " + String(hour()));
     // DEBUG_PRINT(" Minutes: " + String(minute()));
     // DEBUG_PRINTLN(" Seconds: " + String(second()));
      
      
      return true;
    }    
  }
  return true;
}


/******************************************************************************************************************************* */
bool CIRWifi::InitAccessPointMode(void) 
{    
  WiFi.disconnect();
  if (!strcmp(strSSID,""))
  {
    WiFi.mode(WIFI_AP);
  } 
  else
  {
    WiFi.mode(WIFI_AP_STA);
  }
  
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  
  return Connect_Wifi_APMode();
  
  
  return true;
}

/***************************************************************************************************************************************************/
bool CIRWifi::Connect_Wifi_APMode()
{ 
  if (WiFi.status() != WL_CONNECTED)
  { 
    String strSSIDTemp = "ESP_";
    strSSIDTemp += WiFi.macAddress();
    String strPasswwordTemp = "12345678";
    //DEBUG_PRINTLN("Opening SoftAP - SSID: " + strSSIDTemp);
    //DEBUG_PRINT(" PASSPHRASE: ");
    //DEBUG_PRINTLN(strSSIDPassword);
    
    WiFi.softAP(strSSIDTemp.c_str(), strPasswwordTemp.c_str());
    unsigned long now = millis();
    while (millis() - now < 30000) 
    {
    //  DEBUG_PRINT(".");
      //if (WiFi.status() == WL_CONNECTED) 
      //{
        //DEBUG_PRINTLN("");
        Serial.print(F("Local IP: "));
       // DEBUG_PRINTLN(WiFi.localIP());
        Serial.print(F("SoftAP IP: "));
       // DEBUG_PRINTLN(WiFi.softAPIP());
       // DEBUG_PRINTLN("Wifi Reconected!");
        if (byMaxStatePossible < AP_WEBSERVER)
        {
          byMaxStatePossible = AP_WEBSERVER;
        }
        return true;
      //}
      delay(100);
    }
   // DEBUG_PRINTLN("Unable to connect Wifi_APMode " + String(WiFi.status()));
    return false;
  } 
  return true;

}
/******************************************************************************************************************************* */
bool CIRWifi::Connect_Wifi_STAMode()
{
  //DEBUG_PRINTLN("Entrando em Connect_Wifi_STAMode()");

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(F("Attempting to connect to WEP network, SSID: "));
    Serial.println(strSSID);
  //  DEBUG_PRINTLN(strSSIDPassword);
    
    WiFi.begin(strSSID, strSSIDPassword);
    
    unsigned long now = millis();
    while (millis() - now < 10000) 
    {
      Serial.print(F("."));
      if (WiFi.status() == WL_CONNECTED) 
      {
        if (byMaxStatePossible < STA_WEBSERVER)
        {
          byMaxStatePossible = STA_WEBSERVER;
        }
        return true;
      }
      delay(100);
    }
    Serial.print(F("Unable to connect Wifi_STAMode"));
  //  DEBUG_PRINTLN(" erro " + String(WiFi.status()));      
    return false;
  }
  return true;
}

/*********************************************************************************************************************************/
bool CIRWifi::InitMQTT()
{ 
  MQTTClient.setServer(strBrokerURL, atoi(strBrokerPort));
  MQTTClient.setClient(wifiClient);
  return(Connect_MQTT());
}

/*********************************************************************************************************************************/
bool CIRWifi::Connect_MQTT() 
{  
  //DEBUG_PRINTLN("entrando em Connect_MQTT()");
  String strSubscribePath;
  strSubscribePath = strMqttUser;
  strSubscribePath += F("/feeds/");
  strSubscribePath += strDeviceName;
        
  // Loop until we're reconnected
  if (!MQTTClient.connected()) 
  {
    Serial.println(F("Trying MQTT connection"));
   // DEBUG_PRINTLN("Mqtt ID: " + String(strMqttUser));
    // Attempt to connect
    
    unsigned long now = millis();
    while (millis() - now < 10000) 
    {
      Serial.print(F("."));
      MQTTClient.connect(strDeviceName,strMqttUser,strMqttPasscode);    
      delay(1000);
      if (MQTTClient.connected() == true) 
      {
        Serial.println(F("MQTT (Re)conected!"));
        unsigned long now2 = millis();
        while (millis() - now2 < 10000) 
        {
          Serial.print(F("."));
          if (MQTTClient.subscribe(strSubscribePath.c_str()))
          {
            Serial.print(F("MQTT Path Subscribed: "));
            Serial.println(strSubscribePath);
            bMqttOK = true;
            return true;
          }
          delay(1000);
        }
        MQTTClient.disconnect();
        bMqttOK = false;
        return false;
      }
      delay(100);
    }
    
    Serial.println(F("Cant connect MQTT"));
    Serial.println(MQTTClient.state());    
    bMqttOK = false;
    return false;
  }
  return true;
}
/* --------------------------------------------------------------------------------------------------------------------------------- */
void CIRWifi::WelcomePage()
{
        String content;
        //IPAddress ip = WiFi.softAPIP();
        //String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
// add header progmem page
        content = FPSTR(HeaderPage);
        
        
        if (!strcmp(strSSID,""))
        {
          content += F("<label>WiFi:<br> </label><input name='ssid' length=32><br>");
          content += F("<label>Password: <br></label><input type = \"password\" input name='pass' length=64><br>");
        }
        else
        {
          content += "<label>WiFi:<br> </label><input name='ssid' length=32 value=\"" + (String)strSSID + "\"><br>";
          content += "<label>Password: <br></label><input type = \"password\" input name='pass' length=64 value=\"" + (String)strSSIDPassword + "\"><br>";
        }
        if (!strcmp(strMqttUser,""))
        {
          content += F("<label>Usuário MQTT:<br> </label><input type = \"text\" input name='usermqtt' length=64><br>");
        }
        else
        {
          content += "<label>Usuário MQTT:<br> </label><input type = \"text\" input name='usermqtt' length=64 value=\"" + String(strMqttUser) + "\"><br>";
        }

        if (!strcmp(strMqttPasscode,""))
        {
          content += F("<label>Passkey MQTT:<br> </label><input type = \"text\" input name='mqttpasskey' size=\"40\" length=32><br>");
        }
        else
        {
          content += "<label>Passkey MQTT:<br> </label><input type = \"text\" input name='mqttpasskey' size=\"40\" length=32 value=\"" + String(strMqttPasscode) + "\"><br>";
        }
        if (!strcmp(strDeviceName,""))
        {
          content += "<label>Nome Dispositivo:<br> </label><input type = \"text\" input name='device' length=64 value=\"Chave" + String(ESP.getChipId()) +"\"><br>";
        }
        else
        {
          content += "<label>Nome Dispositivo:<br> </label><input type = \"text\" input name='device' length=64 value=\"" + String(strDeviceName) + "\"><br>";
        }
        
        if (!strcmp(strBrokerURL,""))
        {
          content += F("<label>Servidor Mqtt:<br> </label><input type = \"text\" input name='broker' length=64 value=\"iot.eclipse.org\"><br>");
        }
        else
        {
          content += "<label>Servidor Mqtt:<br> </label><input type = \"text\" input name='broker' length=64 value=\"" + String(strBrokerURL) + "\"><br>";
        }
        
        content += F("<label>Porta Servidor Mqtt:<br> </label><input name='brokerport' length=64 value = \"1883\"><br>");
        if (!strcmp(strTimeServer,""))
        {
          content += F("<label>Servidor Time:<br> </label><input name='timeserver' length=64 value = \"a.st1.ntp.br\"><br>");
        }
        else
        {
          content += ("<label>Servidor Time:<br> </label><input name='timeserver' length=64 value = \"" + String(strTimeServer) + "\"><br>");
        }
        content += F("<label>Time Zone:<br> </label>");
        content += F("<select name=\"utc\">");
        content += F("<option value=\"-2\">Brasil / Fernando de Noronha / UTC -2:00 </option>");
        content += F("<option value=\"-3\">Brasil / Sao Paulo / UTC -3:00 </option>");
        content += F("<option value=\"-4\">Brasil / Cuiaba / UTC -4:00 </option>");
        content += F("</select><br><br>");
        content += F("<label>-------------------- POSICIONAMENTO -----------------------<br><br> </label>");
        content += F("<label>API URL:<br> </label>");
        content += F("<select name=\"positionURL\">");
        content += F("<option value=\"1\">www.googleapis.com</option>");
        content += F("</select><br><br>");
        
        if (strPositionKey == "")
        {
          content += F("<label>API Chave: <br></label><input type = \"text\" input name='positionkey' length=64><br>");                                       
        }
        else
        {
          content += "<label>API Chave: <br></label><input type = \"text\" input name='positionkey' size=\"40\" length=64 value=\"" + (String)strPositionKey + "\"><br>";
        }
        content += F("<label><br>------------------------ CLIMA ----------------------------<br><br> </label>");
        content += F("<label>API URL:<br> </label>");
        content += F("<select name=\"weatherURL\">");
        content += F("<option value=\"1\">api.wunderground.com</option>");
        content += F("</select><br><br>");
        
        if (strWeatherKey == "")
        {
          content += F("<label>API Chave: <br></label><input type = \"text\" input name='weatherkey' size=\"40\" length=64><br>");
        }
        else
        {
          content += "<label>API Chave: <br></label><input type = \"text\" input name='weatherkey' size=\"40\" length=64 value=\"" + (String)strWeatherKey + "\"><br>";         
        }    
        content += F("<label><br>------------------ COMANDOS DE VOZ ------------------------<br><br> </label>");
        
        if (!strcmp(strSpeechKey,""))
        {
          content += F("<label>Chave API Sinric:<br> </label><input name='speechkey' size=\"40\" length=40><br><br>");
        }
        else
        {
           content += "<label>Chave API Sinric:<br> </label><input name='speechkey' size=\"40\" length=40 value =\"" + String(strSpeechKey) + "\"><br><br>";
        }
        if (!strcmp(strSpeechDevID,""))
        {
          content += F("<label>Device ID:<br> </label><input name='speechdevid' size=\"40\" length=40><br><br>");
        }
        else
        {
           content += "<label>Device ID:<br> </label><input name='speechdevid' size=\"40\" length=40 value =\"" + String(strSpeechDevID) + "\"><br><br>";
        }
        
        content += F("<label><br>------------------ TV ------------------------<br><br> </label>");
        
        content += F("<label>On/Off..........</label><input type=\"text\" size=\"12\" value=\"nao mapeado\"><input type=\"button\" value=\"(re)mapear\" onclick=\"window.location.href='/btnTVOnOff'\"><br>");

        
        content += F("<label>------------------------ REGRAS DE ATIVACAO ----------------------------<br><br> </label>");
        content += "<p>" + GetExpressions() + " </p>";     
        content += F("<br><br> <input type='submit'>");
        
        content += FPSTR(FooterPage);

        content.replace(F("{{VERSION}}"),VERSION);

    //    DEBUG_PRINTLN(content);
        p_Server->send(200, "text/html", content);
        
}
/* ******************************************************************************************************************************************************* */

void CIRWifi::ClearPage()
{
    String content;
    content = FPSTR(HeaderPage);
    content += F("<p> Modulo Formatado e Resetado!!!!");
    content += FPSTR(FooterPage);
    content.replace(F("{{VERSION}}"),VERSION);
    p_Server->send(200, "text/html", content);
    delay(2000);
    Format();
    Reset(); 
}
/* ******************************************************************************************************************************************************* */

void CIRWifi::BtnTVOnOffPage()
{
  String content;
  content = FPSTR(HeaderPage);
  content += F("<br><br><br><p> Aponte o controle remoto da TV para o sensor e aperte o botão ON/OFF");
  content += FPSTR(FooterPage);
  iButtonIRIndex = IR_TV_ONOFF;
  bIsReceivingIR = true;
  irrecv.enableIRIn();  // Start the receiver
  p_Server->send(200, "text/html", content);
}
/* ******************************************************************************************************************************************************* */
void CIRWifi::SubmitPage()
{
    int iGeoIndex;
    int iWeatherIndex;
    String content;
    strcpy(strSSID,p_Server->arg("ssid").c_str());
    strcpy(strSSIDPassword,p_Server->arg("pass").c_str());
    strcpy(strDeviceName,p_Server->arg("device").c_str());
    strcpy(strMqttUser,p_Server->arg("usermqtt").c_str());
    strcpy(strMqttPasscode,p_Server->arg("mqttpasskey").c_str());
    strcpy(strBrokerURL,p_Server->arg("broker").c_str());
    strcpy(strBrokerPort,p_Server->arg("brokerport").c_str());
    strcpy(strTimeServer,p_Server->arg("timeserver").c_str());
    utc = p_Server->arg("utc").toInt();
    iGeoIndex = p_Server->arg("positionURL").toInt();
    strcpy(strPositionKey,p_Server->arg("positionkey").c_str());
    iWeatherIndex = p_Server->arg("weatherURL").toInt();
    strcpy(strWeatherKey,p_Server->arg("weatherkey").c_str());
    strcpy(strSpeechKey,p_Server->arg("speechkey").c_str());
    strcpy(strSpeechDevID,p_Server->arg("speechdevid").c_str());
    SetPositionURLByIndex(iGeoIndex);
    SetWeatherURLByIndex(iWeatherIndex);

    if (strlen(strSSID) > 0 && strlen(strSSIDPassword) > 0) 
    {
      SaveSettings();
      content = FPSTR(HeaderPage);
      content += F("<p> Dados Salvos na memoria interna. Modulo sendo resetado...");
      content += FPSTR(FooterPage);  
      content.replace(F("{{VERSION}}"),VERSION); 
      p_Server->send(200, "text/html", content);
      delay(4000);
      p_Server->stop();
      p_Server->close();
      ESP.restart();
    } 
    else 
    {
      content = F("{\"Error\":\"404 not found\"}");
   //   DEBUG_PRINTLN("Sending 404");
      p_Server->send(404, "text/html", content);
    }       

}
/* ******************************************************************************************************************************************************* */
String CIRWifi::GetPositionURLByIndex(int iIndexParam)
{
  if (iIndexParam == 1) return F("www.googleapis.com");
  return "";
}
/* ******************************************************************************************************************************************************* */
String CIRWifi::GetWeatherURLByIndex(int iIndexParam)
{
  if (iIndexParam == 1) return F("api.wunderground.com");
  return "";
}

/* ******************************************************************************************************************************************************* */
int CIRWifi::GetIndexByURLPosition()
{
    if (strPositionURL == F("www.googleapis.com")) return 1;
    return 0;
}
/* ******************************************************************************************************************************************************* */
int CIRWifi::GetIndexByURLWeather()
{
    if (strWeatherURL == F("api.wunderground.com")) return 1;
    return 0;
}

/* ******************************************************************************************************************************************************* */
bool CIRWifi::SetWeatherURLByIndex(int iIndexParam)
{
      if (iIndexParam == 1)
      {
        strWeatherURL = F("api.wunderground.com");
        return true;
      }
    return false;
}
/* ******************************************************************************************************************************************************* */
bool CIRWifi::SetPositionURLByIndex(int iIndexParam)
{
  if (iIndexParam == 1)
  {
    strPositionURL = F("www.googleapis.com"); 
    return true;
  }
  return false;
}

/* ******************************************************************************************************************************************************* */
bool CIRWifi::InitWebServer()
{
 // DEBUG_PRINTLN("entrando em InitWebSerever()");
  String strDeviceParam;    
  if (p_Server == NULL)
  {
    IPAddress IPadd;    
    if (byState == AP_WEBSERVER)
    {
      IPadd = WiFi.softAPIP();
    }
    if (byState == STA_WEBSERVER)
    {
      IPadd = WiFi.localIP();
    }
    p_Server = new ESP8266WebServer(80);
    if (p_Server)
    {
      strDeviceParam = F("Chave");
      if (strDeviceName != "")
      {
         strDeviceParam = strDeviceName;
      }
          
      if (mdns.begin(strDeviceParam.c_str(),IPadd))
      {
        Serial.println(F("mdns started!"));
      }
      else
      {
        Serial.println(F("mdns failed!"));
      }
       httpUpdater.setup(p_Server);

      p_Server->on("/", std::bind(&CIRWifi::WelcomePage, this)); 
      p_Server->on("/clear", std::bind(&CIRWifi::ClearPage, this));
      p_Server->on("/setting",std::bind(&CIRWifi::SubmitPage, this));
      p_Server->on("/btnTVOnOff",std::bind(&CIRWifi::BtnTVOnOffPage, this));  
      p_Server->begin();
      mdns.addService("http","tcp",80); 
    }
  }
  return true;
}

//--------------------------------------------------------------------------------------------

bool CIRWifi::EndWebServer()
{     
  if (p_Server)
  {
    p_Server->close();
    delete p_Server;
    p_Server = NULL;
  }
}

//--------------------------------------------------------------------------------------------

bool CIRWifi::InitStationMode()
{
  unsigned int i;
  unsigned int j;
  //Credentials for Google GeoLocation API...
  String thisPage = F("/geolocation/v1/geolocate?key=");
  int iMatches = 0;
  int n;
  bool bNeedsGeoUpdate = false;
  String jsonString = "{\n";
  DynamicJsonBuffer jsonBuffer;
//  DEBUG_PRINTLN("Entrando em InitStationMode()");

  if (WiFi.getMode() == WIFI_AP)
  {
    WiFi.mode(WIFI_AP_STA);
  } 

   if (Connect_Wifi_STAMode()) 
      {
     //   DEBUG_PRINTLN("");
        Serial.println(F("WiFi connected!"));
      //  DEBUG_PRINT("Local IP: ");
      //  DEBUG_PRINTLN(WiFi.localIP());
      //  DEBUG_PRINT("SoftAP IP: ");
      //  DEBUG_PRINTLN(WiFi.softAPIP());
        
        Serial.println(F("Scanning WiFi Nets"));
        // WiFi.scanNetworks will return the number of networks found
        n = WiFi.scanNetworks();
        if (n == 0)
        {
          Serial.println(F("Nearby geo networks not found"));
        }
        else
        {
          if (n > NEARBY_SSID_MAX_REGS)
          {
            n = NEARBY_SSID_MAX_REGS;
          }
          // is needed update geopositioning?
             
          for (i = 0; i < n; i++)
           {
            // Print SSID and RSSI for each network found
        //    DEBUG_PRINT(i + 1);
        //    DEBUG_PRINT(": ");
        ///    DEBUG_PRINT(WiFi.SSID(i));
        //    DEBUG_PRINT(" (");
       //     DEBUG_PRINT(WiFi.RSSI(i));
       //     DEBUG_PRINT(")");
        //    DEBUG_PRINTLN((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
            delay(10);
            for (j = 0; j < NearByWifi.size();j++)
            {
              if (WiFi.SSID(i) == NearByWifi[j])
              {
                Serial.print(F("Matches between nearby SSID/memory GeoPos:"));
                Serial.println(NearByWifi[j]);
                iMatches++;
                break;
              }
            }
           }
      
          if (iMatches == 0)
          {
            Serial.println(F("0 Matches found, updating geopos"));
            bNeedsGeoUpdate = true;
          }
          else
          {
            Serial.print(F("Total Memory Nearby GeosPos:"));
            Serial.println(String(NearByWifi.size()));
            Serial.print(F("Total matches memory with Wifi SSIDs:"));
            Serial.println(String(iMatches));
            Serial.print(F("Nearby SSID nets match ratio:"));
            Serial.println(String((float)iMatches / (float)NearByWifi.size()));
            if ((((float)iMatches / (float)NearByWifi.size()) < GEO_MATCHES_RATIO) || (dbLatitude == 0.0f && dbLongitude == 0.0f))
            {
              Serial.println(F("need to update geopos"));      
              bNeedsGeoUpdate = true;
            }
            else
            {
              Serial.println(F("no neeed upd geopos, using memory pos"));      
              bNeedsGeoUpdate = false; 
              bGeoKnown = true;
            }
          }     
        }
        if (bNeedsGeoUpdate && strPositionKey != "")
        {
          NearByWifi.clear();
          for (i = 0; i < n; i++)
          {
        //    DEBUG_PRINTLN("rede inserida:" + String(WiFi.SSID(i)) + " - " + String(WiFi.BSSIDstr(i)));
            NearByWifi.push_back(WiFi.SSID(i));
          }
          SaveNearbySSIDs();
          jsonString="{\n";
          jsonString +=F("\"homeMobileCountryCode\": 724,\n");  // this is a real UK MCC
          jsonString +=F("\"homeMobileNetworkCode\": 11,\n");   // and a real UK MNC
          jsonString +=F("\"radioType\": \"gsm\",\n");          // for gsm
          jsonString +=F("\"carrier\": \"Claro\",\n");       // associated with Vodafone 
          jsonString +=F("\"wifiAccessPoints\": [\n");
          for (j = 0; j < n; ++j)
          {
            jsonString +=F("{\n");
            jsonString +=F("\"macAddress\" : \"");    
            jsonString +=(WiFi.BSSIDstr(j));
            jsonString +=F("\",\n");
            jsonString +=F("\"signalStrength\": ");     
            jsonString +=WiFi.RSSI(j);
            jsonString +=F("\n");
            if(j<n-1)
            {
              jsonString +=F("},\n");
            }
            else
            {
              jsonString +=F("}\n");  
            }
          }
          jsonString +=("]\n");
          jsonString +=("}\n"); 
         // DEBUG_PRINTLN("jsonBuffer:");
        //  DEBUG_PRINTLN(jsonString);
        //  DEBUG_PRINTLN("Connecting Geo Server: " + strPositionURL);
          WiFiClientSecure client;
          if (client.connect(strPositionURL.c_str(), 443)) 
          {
         //   DEBUG_PRINTLN("Geo Server Connected!");    
            client.println("POST " + thisPage + strPositionKey + " HTTP/1.1");    
            client.println("Host: "+ strPositionURL);
            client.println(F("Connection: close"));
            client.println(F("Content-Type: application/json"));
            client.println(F("User-Agent: Arduino/1.0"));
            client.print(F("Content-Length: "));
            client.println(jsonString.length());    
            client.println();
            client.print(jsonString);  
            delay(500);
  
            //Read and parse all the lines of the reply from server          
            while (client.available()) 
            {
              String line = client.readStringUntil('\r');
        //      DEBUG_PRINT(line);
                  
              JsonObject& root = jsonBuffer.parseObject(line);
              if(root.success())
              {
              dbLatitude    = root["location"]["lat"];
              dbLongitude   = root["location"]["lng"];
              bGeoKnown = true;
              }
            }
  
         //   DEBUG_PRINTLN("closing geo server connection");
         //   DEBUG_PRINTLN();
            client.stop();
          
         //   DEBUG_PRINT("Latitude = ");
        //    DEBUG_PRINTFORMATTED(dbLatitude,6);
        //    DEBUG_PRINT("Longitude = ");
        //    DEBUG_PRINTFORMATTED(dbLongitude,6);
            SaveGeoPositioning();
           }
        }
      //  DEBUG_PRINTLN("Disabling AP Mode");
        WiFi.mode(WIFI_STA);
        WiFi.softAPdisconnect(true);
        return true;
      }
      return false;
}


/***************************************************************************************************************************************************/

void CIRWifi::LoadSettings()
{
  unsigned int i;
  unsigned int j;
  char chChar; 
  byte byBufferSize = 0;

  for (i = HEADER_INIT; i < HEADER_INIT + HEADER_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strHeader[byBufferSize] = chChar;
      byBufferSize++;
      strHeader[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  

  Serial.print(F("Header:"));
  Serial.println(strHeader);

  if (strcmp(strHeader,VERSION))
  {
    Serial.println(F("unrecognized header, formatting..."));
    Format();
  }

  byBufferSize = 0;
  for (i = SSID_MEM_INIT; i < SSID_MEM_INIT + SSID_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strSSID[byBufferSize] = chChar;
      byBufferSize++;
      strSSID[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }

  Serial.print(F("SSID:"));
  Serial.println(strSSID);

  byBufferSize = 0;
  for (i = SSIDPASSWORD_MEM_INIT; i < SSIDPASSWORD_MEM_INIT + SSIDPASSWORD_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strSSIDPassword[byBufferSize] = chChar;
      byBufferSize++;
      strSSIDPassword[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }  

  Serial.print(F("PASS:"));
  Serial.println(strSSIDPassword);

  byBufferSize = 0;
  for (i = DEVICENAME_MEM_INIT; i < DEVICENAME_MEM_INIT + DEVICENAME_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strDeviceName[byBufferSize] = chChar;
      byBufferSize++;
      strDeviceName[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  Serial.print(F("Device Name:"));
  Serial.println(strDeviceName);   

  byBufferSize = 0;
  for (i = MQTTUSER_MEM_INIT; i < MQTTUSER_MEM_INIT + MQTTUSER_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strMqttUser[byBufferSize] = chChar;
      byBufferSize++;
      strMqttUser[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  Serial.print(F("UserName:"));
  Serial.println(strMqttUser);

   byBufferSize = 0;
  for (i = MQTTPASSCODE_MEM_INIT; i < MQTTPASSCODE_MEM_INIT + MQTTPASSCODE_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strMqttPasscode[byBufferSize] = chChar;
      byBufferSize++;
      strMqttPasscode[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  Serial.print(F("Passcode:"));
  Serial.println(strMqttPasscode);


  byBufferSize = 0;
  for (i = BROKERURL_MEM_INIT; i < BROKERURL_MEM_INIT + BROKERURL_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strBrokerURL[byBufferSize] = chChar;
      byBufferSize++;
      strBrokerURL[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  Serial.print(F("Mqtt Server:"));
  Serial.println(strBrokerURL);   

  byBufferSize = 0;
  for (i = BROKERPORT_MEM_INIT; i < BROKERPORT_MEM_INIT + BROKERPORT_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strBrokerPort[byBufferSize] = chChar;
      byBufferSize++;
      strBrokerPort[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  Serial.print(F("Mqtt Port:"));
  Serial.println(strBrokerPort);   

  byBufferSize = 0;
  for (i = TIMESERVER_MEM_INIT; i < TIMESERVER_MEM_INIT + TIMESERVER_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strTimeServer[byBufferSize] = chChar;
      byBufferSize++;
      strTimeServer[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  Serial.print(F("Time Server:"));
  Serial.println(strTimeServer);
  
  EEPROM_readAnything(GEO_POS_URL_MEM_INIT,i); 
  strPositionURL = GetPositionURLByIndex(i);
//  DEBUG_PRINT("Geo Position URL: ");
  //  DEBUG_PRINTLN(strPositionURL);
  
  byBufferSize = 0;
  for (i = GEO_POS_KEY_MEM_INIT; i < GEO_POS_KEY_MEM_INIT + GEO_POS_KEY_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strPositionKey[byBufferSize] = chChar;
      byBufferSize++;
      strPositionKey[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  Serial.print(F("Geo Position Key:"));
  Serial.println(strPositionKey);

  EEPROM_readAnything(GEO_LAT_MEM_INIT,dbLatitude);
  EEPROM_readAnything(GEO_LONG_MEM_INIT,dbLongitude);
  
  EEPROM_readAnything(WEATHER_URL_MEM_INIT,i); 
  strWeatherURL = GetWeatherURLByIndex(i);
//  DEBUG_PRINT("Weather URL: ");
//  DEBUG_PRINTLN(strWeatherURL);
  
  byBufferSize = 0;
  for (i = WEATHER_KEY_MEM_INIT; i < WEATHER_KEY_MEM_INIT + WEATHER_KEY_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strWeatherKey[byBufferSize] = chChar;
      byBufferSize++;
      strWeatherKey[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  Serial.print(F("Weather Key:"));
  Serial.println(strWeatherKey);

  byBufferSize = 0;
  for (i = SPEECH_KEY_MEM_INIT; i < SPEECH_KEY_MEM_INIT + SPEECH_KEY_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strSpeechKey[byBufferSize] = chChar;
      byBufferSize++;
      strSpeechKey[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  Serial.print(F("Speech API Key:"));
  Serial.println(strSpeechKey);

  byBufferSize = 0;
  for (i = SPEECH_DEVICE_ID_MEM_INIT; i < SPEECH_DEVICE_ID_MEM_INIT + SPEECH_DEVICE_ID_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strSpeechDevID[byBufferSize] = chChar;
      byBufferSize++;
      strSpeechDevID[byBufferSize] = '\0';
    }
    else
    {
      break;
    }
  }
  Serial.print(F("Speech Dev ID:"));
  Serial.println(strSpeechDevID);
  
  byte bySize;
  EEPROM_readAnything(NEARBY_SSID_QUANT_MEM_INIT,bySize);
  Serial.print(F("Loading Nearby SSIDs count: "));
  Serial.println(String(bySize));
    
  for (i = 0; i < bySize;i++)
  {
     String strNearbyWifi;
     for (j = NEARBY_SSID_MEM_INIT + (i * NEARBY_SSID_REG_SIZE); j < NEARBY_SSID_MEM_INIT + (i * NEARBY_SSID_REG_SIZE) + NEARBY_SSID_REG_SIZE ;j++)
     {
        char chRead = char(EEPROM.read(j));
        if (chRead != '\0')
        {
          strNearbyWifi += chRead;
        }
        else 
        {
          break;
        }
      }
      NearByWifi.push_back(strNearbyWifi);  
  //    DEBUG_PRINTLN(strNearbyWifi);
  }  

  // IR saved buttons
  
  EEPROM_readAnything(IR_TV_ONOFF_FREQ_MEM_INIT,IRButtons[IR_TV_ONOFF].uiFreq);
  Serial.print(F("TV ON/OFF Freq: "));
  Serial.println(IRButtons[IR_TV_ONOFF].uiFreq);
  EEPROM_readAnything(IR_TV_ONOFF_SIZE_MEM_INIT,IRButtons[IR_TV_ONOFF].uiSize);
  Serial.print(F("TV ON/OFF RawSize: "));
  Serial.println(IRButtons[IR_TV_ONOFF].uiSize);
  if (IRButtons[IR_TV_ONOFF].uiSize)
  {
    IRButtons[IR_TV_ONOFF].p_uiRawData = new uint16_t[ IRButtons[IR_TV_ONOFF].uiSize];    
    int j=0;
    Serial.print(F("TV ON/OFF Raw[]: "));
    for (i = IR_TV_ONOFF_RAW_MEM_INIT; i < (IR_TV_ONOFF_RAW_MEM_INIT + IRButtons[IR_TV_ONOFF].uiSize * sizeof(uint16_t)); i+=sizeof(uint16_t))
    {
     EEPROM_readAnything(i,IRButtons[IR_TV_ONOFF].p_uiRawData[j]);
     Serial.print(IRButtons[IR_TV_ONOFF].p_uiRawData[j]);
     Serial.print(" ");
     
     j++;
    }
    Serial.println(" "); 
  }

  EEPROM_readAnything(IR_TV_INPUT_FREQ_MEM_INIT,IRButtons[IR_TV_INPUT].uiFreq);
  EEPROM_readAnything(IR_TV_INPUT_SIZE_MEM_INIT,IRButtons[IR_TV_INPUT].uiSize);  
  if (IRButtons[IR_TV_INPUT].uiSize)
  {
    IRButtons[IR_TV_INPUT].p_uiRawData = new uint16_t[ IRButtons[IR_TV_INPUT].uiSize];  
    int j=0;
    for (i = IR_TV_INPUT_RAW_MEM_INIT; i < (IR_TV_INPUT_RAW_MEM_INIT + IRButtons[IR_TV_INPUT].uiSize * sizeof(uint16_t)); i+=sizeof(uint16_t))
    {
     EEPROM_readAnything(i,IRButtons[IR_TV_INPUT].p_uiRawData[j]);
     j++;
    }   
  }

  EEPROM_readAnything(IR_TV_VOLUP_FREQ_MEM_INIT,IRButtons[IR_TV_VOLUP].uiFreq);
  EEPROM_readAnything(IR_TV_VOLUP_SIZE_MEM_INIT,IRButtons[IR_TV_VOLUP].uiSize);  
  if (IRButtons[IR_TV_VOLUP].uiSize)
  {
    IRButtons[IR_TV_VOLUP].p_uiRawData = new uint16_t[ IRButtons[IR_TV_VOLUP].uiSize];   
    int j=0;
    for (i = IR_TV_VOLUP_RAW_MEM_INIT; i < (IR_TV_VOLUP_RAW_MEM_INIT + IRButtons[IR_TV_VOLUP].uiSize * sizeof(uint16_t)); i+=sizeof(uint16_t))
    {
     EEPROM_readAnything(i,IRButtons[IR_TV_VOLUP].p_uiRawData[j]);
     j++;
    }  
  }

  
  
  LoadExpressions(MEM_INIT_EXPRESSION_POINTER);
  
}


/*********************************************************************************************************************************************************/
void CIRWifi::SaveSettings()
{
  unsigned int i;
  
  for (int i = HEADER_INIT + HEADER_SIZE; i < MEM_SIZE; ++i) { EEPROM.write(i, 0); }

 
//  DEBUG_PRINTLN(strSSID);
  for (i = 0; i < strlen(strSSID); i++)
    {
      EEPROM.write(SSID_MEM_INIT + i, strSSID[i]);
    }
//  DEBUG_PRINTLN(strSSIDPassword);
  for (i = 0; i < strlen(strSSIDPassword); i++)
    {
      EEPROM.write(SSIDPASSWORD_MEM_INIT + i, strSSIDPassword[i]);
    }  
//  DEBUG_PRINTLN(strDeviceName);
  for (i = 0; i < strlen(strDeviceName); i++)
  {
      EEPROM.write(DEVICENAME_MEM_INIT + i, strDeviceName[i]);
  }
  
 // DEBUG_PRINTLN(strMqttUser);
  for (i = 0; i < strlen(strMqttUser); i++)
  {
      EEPROM.write(MQTTUSER_MEM_INIT + i, strMqttUser[i]);
  }
  
//  DEBUG_PRINTLN(strMqttPasscode);
  for (i = 0; i < strlen(strMqttPasscode); i++)
  {
      EEPROM.write(MQTTPASSCODE_MEM_INIT + i, strMqttPasscode[i]);
  }
  
  
 // DEBUG_PRINTLN(strBrokerURL);
  for (i = 0; i < strlen(strBrokerURL); i++)
  {
      EEPROM.write(BROKERURL_MEM_INIT + i, strBrokerURL[i]);
  }
 // DEBUG_PRINTLN(strBrokerPort);
  for (i = 0; i < strlen(strBrokerPort); i++)
  {
      EEPROM.write(BROKERPORT_MEM_INIT + i, strBrokerPort[i]);
  }
//  DEBUG_PRINTLN(strTimeServer);
  for (i = 0; i < strlen(strTimeServer);i++)
  {
      EEPROM.write(TIMESERVER_MEM_INIT + i, strTimeServer[i]);
  }
  
  EEPROM_writeAnything(GEO_POS_URL_MEM_INIT,GetIndexByURLPosition()); 
//  DEBUG_PRINT("Geo Position URL: ");
 // DEBUG_PRINTLN(strPositionURL);
  
 // DEBUG_PRINTLN(strPositionKey);
  for (i = 0; i < strlen(strPositionKey);i++)
  {
      EEPROM.write(GEO_POS_KEY_MEM_INIT + i, strPositionKey[i]);
  }
  
  SaveGeoPositioning();

  EEPROM_writeAnything(WEATHER_URL_MEM_INIT,GetIndexByURLWeather()); 
//  DEBUG_PRINT("Weather URL: ");
//  DEBUG_PRINTLN(strWeatherURL);
  
//  DEBUG_PRINTLN(strWeatherKey);
  for (i = 0; i < strlen(strWeatherKey);i++)
  {
      EEPROM.write(WEATHER_KEY_MEM_INIT + i, strWeatherKey[i]);
  }
  
//  DEBUG_PRINTLN(strSpeechKey);
  for (i = 0; i < strlen(strSpeechKey);i++)
  {
      EEPROM.write(SPEECH_KEY_MEM_INIT + i, strSpeechKey[i]);
  }

 //  DEBUG_PRINTLN(strSpeechDevID);
  for (i = 0; i < strlen(strSpeechDevID);i++)
  {
      EEPROM.write(SPEECH_DEVICE_ID_MEM_INIT + i, strSpeechDevID[i]);
  }
  
  SaveNearbySSIDs();

  SaveIRButtons();

  

  SaveExpressions(MEM_INIT_EXPRESSION_POINTER);

  EEPROM.commit();
}
/*********************************************************************************************************************************************************/
void CIRWifi::LoadExpressions(int iInitMemoryParam)
{
  unsigned int i;
  unsigned int j;
  int iQtdExpressions;
  int iQtdSensors;
  
  // le a quantidade de expressoes
  EEPROM_readAnything(iInitMemoryParam,iQtdExpressions);
  iInitMemoryParam += sizeof(int);
 
  for (i = 0; i < iQtdExpressions; i++)
  {
    // le a quantidade de sensores na expressao
    int iQtdSensors;
    EEPROM_readAnything(iInitMemoryParam,iQtdSensors);
    iInitMemoryParam += sizeof(int);
    // le o tipo de sensor
    int iTypeSensor;

    stExpressionItem *p_ExpItem;

    p_ExpItem = new stExpressionItem;
    Expression.push_back(p_ExpItem);
    
    for (j = 0; j < iQtdSensors; j++)
    {
       CEventHandler *p_Event;
       
       EEPROM_readAnything(iInitMemoryParam,iTypeSensor);
       iInitMemoryParam += sizeof(iTypeSensor);
       if (iTypeSensor == EVT_PRD_HANDLER)
       {
        CEventPeriodHandler *p_PrdHandler;
        p_PrdHandler = new CEventPeriodHandler(0,0,0,0);
        p_Event = (CEventHandler *)p_PrdHandler;
       }
       if (iTypeSensor == EVT_GPS_HANDLER)
       {
        CEventGPSHandler *p_GPSHandler;
        p_GPSHandler = new CEventGPSHandler(this,0,0);
        p_Event = (CEventHandler *)p_GPSHandler;
       }
       if (iTypeSensor == EVT_WTR_HANDLER)
       {
        CEventWeatherHandler *p_WtrHandler;
        p_WtrHandler = new CEventWeatherHandler(this,0,0,0,"");
        p_Event = (CEventHandler *)p_WtrHandler;
       }
       p_Event->Load(&iInitMemoryParam);
       Expression[i]->Sensors.push_back(p_Event);
    }
    
    for (j = 0; j < (iQtdSensors - 1); j++)
    {
      char chOperator;
      chOperator = (char)EEPROM.read(iInitMemoryParam);
      Expression[i]->CauseOperators.push_back(chOperator);
      iInitMemoryParam++;
    }
    
    for (j = 0; j < 4 ; j++)
    {
      Expression[i]->strEffectOperator[j] = char(EEPROM.read(iInitMemoryParam + j));
      if (!isGraph(Expression[i]->strEffectOperator[j]))
      {
        Expression[i]->strEffectOperator[j] = '\0';
        break;
      }
    }
    iInitMemoryParam+=4;    
  }

}
/*********************************************************************************************************************************************************/
void CIRWifi::SaveExpressions(int iInitMemoryParam)
{
  unsigned int i;
  unsigned int j;
  
  for (i = iInitMemoryParam ; i < MEM_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  // salva a quantidade de expressoes
  EEPROM_writeAnything(iInitMemoryParam,(int)Expression.size());
  iInitMemoryParam += sizeof(int);
  for (i = 0; i < Expression.size(); i++)
  {
    // salva quantidade de sensores de cada expressao
    EEPROM_writeAnything(iInitMemoryParam,(int)Expression[i]->Sensors.size());
    iInitMemoryParam += sizeof(int);
    // salva tipos de cada sensor
    for (j = 0; j < Expression[i]->Sensors.size(); j++)
    {
      int iTypeSensor = (int)Expression[i]->Sensors[j]->GetEventType();
       EEPROM_writeAnything(iInitMemoryParam,iTypeSensor);
       iInitMemoryParam += sizeof(iTypeSensor);
    }
    // salva dados dos sensores
    for (j = 0; j < Expression[i]->Sensors.size(); j++)
    {
       Expression[i]->Sensors[j]->Save(&iInitMemoryParam);
    }
    // salva operadores
    for (j = 0; j < Expression[i]->CauseOperators.size(); j++)
    {
      EEPROM.write(iInitMemoryParam, Expression[i]->CauseOperators[j]);
      iInitMemoryParam++;
    }
    // salva operadores de efeito (igualdade/desigualdade)
    for (j = 0; j < 4 ; j++)
    {
      if (isGraph(Expression[i]->strEffectOperator[j]))
      {
        EEPROM.write(iInitMemoryParam + j, Expression[i]->strEffectOperator[j]);
      }
      else
      {
        Expression[i]->strEffectOperator[j] = '\0';
        break;
      }
    }
    iInitMemoryParam+=4;    
  
  }
  EEPROM.commit();

}
/*********************************************************************************************************************************************************/
void CIRWifi::SaveGeoPositioning()
{
  EEPROM_writeAnything(GEO_LAT_MEM_INIT,dbLatitude);
  EEPROM_writeAnything(GEO_LONG_MEM_INIT,dbLongitude);
  EEPROM.commit();  
}
/*********************************************************************************************************************************************************/
void CIRWifi::SaveNearbySSIDs()
{
  unsigned int i;
  unsigned int j;
  
  for (i = NEARBY_SSID_MEM_INIT ; i < NEARBY_SSID_MEM_INIT + (NEARBY_SSID_REG_SIZE * NEARBY_SSID_MAX_REGS); i++)
  {
    EEPROM.write(i, 0);
  }
//  DEBUG_PRINTLN("Saving Nearby SSIDs count: " + String(NearByWifi.size()));
  EEPROM_writeAnything(NEARBY_SSID_QUANT_MEM_INIT,(byte)NearByWifi.size());
    
  for (i = 0; i < NearByWifi.size();i++)
  {
      for (j = 0; j < NearByWifi[i].length();j++)
      {
        EEPROM.write(NEARBY_SSID_MEM_INIT + (i * NEARBY_SSID_REG_SIZE) + j, NearByWifi[i][j]);
      }     
  }
  EEPROM.commit();
}

/* *************************************************************************************************************************************************************************************************************************/
void CIRWifi::Reset()
{
  ESP.restart();
}

/* *************************************************************************************************************************************************************************************************************************/
void CIRWifi::SaveIRButtons()
{
  int i;
  
  // IR saved buttons
  EEPROM_writeAnything(IR_TV_ONOFF_FREQ_MEM_INIT,IRButtons[IR_TV_ONOFF].uiFreq);
  EEPROM_writeAnything(IR_TV_ONOFF_SIZE_MEM_INIT,IRButtons[IR_TV_ONOFF].uiSize);  
  if (IRButtons[IR_TV_ONOFF].uiSize)
  {
    int j=0;
    for (i = IR_TV_ONOFF_RAW_MEM_INIT; i < (IR_TV_ONOFF_RAW_MEM_INIT + IRButtons[IR_TV_ONOFF].uiSize * sizeof(uint16_t)); i+=sizeof(uint16_t))
    {
      EEPROM_writeAnything(i,IRButtons[IR_TV_ONOFF].p_uiRawData[j]);
      j++;
    }    
  }

  EEPROM_writeAnything(IR_TV_INPUT_FREQ_MEM_INIT,IRButtons[IR_TV_INPUT].uiFreq);
  EEPROM_writeAnything(IR_TV_INPUT_SIZE_MEM_INIT,IRButtons[IR_TV_INPUT].uiSize);  
  if (IRButtons[IR_TV_INPUT].uiSize)
  {
    int j=0;
    for (i = IR_TV_INPUT_RAW_MEM_INIT; i < (IR_TV_INPUT_RAW_MEM_INIT + IRButtons[IR_TV_INPUT].uiSize * sizeof(uint16_t)); i+=sizeof(uint16_t))
    {
      EEPROM_writeAnything(i,IRButtons[IR_TV_INPUT].p_uiRawData[j]);
      j++;
    }   
  }

  EEPROM_writeAnything(IR_TV_VOLUP_FREQ_MEM_INIT,IRButtons[IR_TV_VOLUP].uiFreq);
  EEPROM_writeAnything(IR_TV_VOLUP_SIZE_MEM_INIT,IRButtons[IR_TV_VOLUP].uiSize);  
  if (IRButtons[IR_TV_VOLUP].uiSize)
  {
    int j=0;
    for (i = IR_TV_VOLUP_RAW_MEM_INIT; i < (IR_TV_VOLUP_RAW_MEM_INIT + IRButtons[IR_TV_VOLUP].uiSize * sizeof(uint16_t)); i+=sizeof(uint16_t))
    {
      EEPROM_writeAnything(i,IRButtons[IR_TV_VOLUP].p_uiRawData[j]);
      j++;
    }  
  }
  EEPROM.commit();
}

/* *************************************************************************************************************************************************************************************************************************/
bool CIRWifi::ChangeState(byte byNewState)
{
  bool bOk = false;
  if (byState == byNewState) return false;
  
  switch(byNewState)
  {
    case AP_WEBSERVER:
   // DEBUG_PRINTLN("changestate : tentando entrar em AP_WEBSERVER");
      
      if (byState == NONE) // inicializando
      {
        if (InitAccessPointMode())
        {
          bOk = InitWebServer();
        }
      }
      if (byState == STA_WEBSERVER)
      {
        bOk = InitAccessPointMode();
      }
        break;
    case STA_WEBSERVER:
   // DEBUG_PRINTLN("changestate : tentando entrar em STA_WEBSERVER");
      if (byState == NONE)
      {
        if (InitStationMode())
        {
          bOk = InitWebServer();
        }
      }
      if (byState == AP_WEBSERVER)
      {
        bOk = InitStationMode();
      }
      if (bOk)
      {
        bMqttOK = InitMQTT();
        InitSpeech();
      }
      break;                        
  }
  
    if (bOk) 
    {
      byState = byNewState;
      //DEBUG_PRINTLN("New state (1 - NONE  2 - AP_WEBSERVER  3 - STA_WEBSERVER) --> " + String(byNewState));
    }
    else
    {
      //DEBUG_PRINTLN("Failed to change state (1 - NONE  2 - AP_WEBSERVER  3 - STA_WEBSERVER) --> " + String(byNewState));
    }
  return bOk;
}

/* *************************************************************************************************************************************************************************************************************************/
void CIRWifi::SetUserPosition(String strNameParam,double dbLatitudeParam,double dbLongitudeParam)
{
  unsigned int i;
  unsigned int j;
  
  for (i = 0; i < Expression.size(); i++)
  {
    for (j = 0 ; j < Expression[i]->Sensors.size(); j++)
    {
      if (Expression[i]->Sensors[j]->GetEventType()== EVT_GPS_HANDLER)
      {
        ((CEventGPSHandler *)Expression[i]->Sensors[j])->SetClientPosition(strNameParam,dbLatitudeParam,dbLongitudeParam);
      }
    }
  }
}
/* *************************************************************************************************************************************************************************************************************************/
void CIRWifi:: InitSpeech()
{
  // server address, port and URL
  webSocket.begin(F("iot.sinric.com"), 80, "/");
  webSocket.setAuthorization("apikey", strSpeechKey);
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);
}
/* *************************************************************************************************************************************************************************************************************************/
bool CIRWifi::IsDayLightSaving()
{
  if (utc == -3)
  {
     int previousSunday = day() - weekday();
     if (month() < 2 || month() > 10) return true;
     else if (month() == 2 && previousSunday < 14) return true;
     else if (month() == 10 && previousSunday >= 14) return true;
     else return false;
  }
  if ((month()<3) || (month()>10)) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
  if ((month()>3) && (month()<10)) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if (month() == 3 && (hour() + 24 * day()) >= (1 + utc + 24 * (31 - (5 * year() / 4 + 4) % 7)) || month() == 10 && (hour() + 24 * day())<(1 + utc + 24 * (31 - (5 * year() / 4 + 1) % 7)))
    return true;
  else
    return false;
}
// ***********************************************************************************************************************************************************************************
//// AND/OR/TRUE_ENABLE/TRUE_DISABLE/TRUE_ENABLE_ELSE/TRUE_DISABLE_ELSE/TRUE_SENSOR
bool CIRWifi::AddEventHandler(eEventHandler enEvtParam,String strPayLoadParam)
{
  CEventHandler *p_Event = NULL;
  int iHourMinuteIni;
  int iHourMinuteEnd;
  int iIndexExpression;
  byte bDay = 0;

  if (enEvtParam == EVT_PRD_HANDLER)
  {
    iIndexExpression = strPayLoadParam.substring(3, 4).toInt();
    if (strPayLoadParam.substring(4, 5) == "1") bDay = DOMINGO;
    if (strPayLoadParam.substring(5, 6) == "1") bDay |= SEGUNDA;
    if (strPayLoadParam.substring(6, 7) == "1") bDay |= TERCA;
    if (strPayLoadParam.substring(7, 8) == "1") bDay |= QUARTA;
    if (strPayLoadParam.substring(8, 9) == "1") bDay |= QUINTA;
    if (strPayLoadParam.substring(9, 10) == "1") bDay |= SEXTA;
    if (strPayLoadParam.substring(10,11) == "1") bDay |= SABADO;
    iHourMinuteIni = strPayLoadParam.substring(11, 15).toInt();
    iHourMinuteEnd = strPayLoadParam.substring(15, 19).toInt();
    CEventPeriodHandler *p_PeriodHandler;
    p_PeriodHandler = new CEventPeriodHandler(1,bDay, iHourMinuteIni,iHourMinuteEnd);
    p_Event = p_PeriodHandler;
  }
  if (enEvtParam == EVT_GPS_HANDLER)
  {
    CEventGPSHandler *p_GPSHandler;
    iIndexExpression = strPayLoadParam.substring(3, 4).toInt();

    String strUser;

    strUser = strPayLoadParam.substring(7, 17);
    
    p_GPSHandler = new CEventGPSHandler(this,1,strPayLoadParam.substring(4, 7).toFloat());
    SetUserPosition(strUser,strPayLoadParam.substring(17, 25).toFloat(),strPayLoadParam.substring(25, 33).toFloat());
    p_Event = p_GPSHandler;
  }
  if (enEvtParam == EVT_WTR_HANDLER)
  {
    if (IsGeoKnown() && GetWeatherAPIKey() != "")
    {
      CEventWeatherHandler *p_WeatherHandler;
      
      iIndexExpression = strPayLoadParam.substring(3, 4).toInt();
    

      p_WeatherHandler = new CEventWeatherHandler(this, strPayLoadParam.substring(4, 7).toFloat(), strPayLoadParam.substring(7, 10).toFloat() / 10.0f, strPayLoadParam.substring(10, 13).toFloat() / 10.0f, strPayLoadParam.substring(13, 17));
      p_Event = p_WeatherHandler;
    }
    else
    {
      return false;
    }
  }

  if (iIndexExpression < Expression.size())
  {
    if (Expression[iIndexExpression]->Sensors.size() > 0)
    {
      Expression[iIndexExpression]->CauseOperators.push_back('A');
      Expression[iIndexExpression]->Sensors.push_back(p_Event);     
    }
  }
  else
  {
    stExpressionItem *p_ExpressionItem;

    p_ExpressionItem = new stExpressionItem;

    p_ExpressionItem->Sensors.push_back(p_Event);
    if (Expression.size() == 0)
    {
      strcpy(p_ExpressionItem->strEffectOperator,"TED"); // TE - TRUE_ENABLE/TD - TRUE_DISABLE/TED - TRUE_ENABLE_ELSE/TDE - TRUE_DISABLE_ELSE   
    }
    else
    {
      strcpy(p_ExpressionItem->strEffectOperator,"TE"); // TE - TRUE_ENABLE/TD - TRUE_DISABLE/TED - TRUE_ENABLE_ELSE/TDE - TRUE_DISABLE_ELSE
    }
    
    Expression.push_back(p_ExpressionItem);    
  }
  SaveExpressions(MEM_INIT_EXPRESSION_POINTER);
  return true;
}

// ***********************************************************************************************************************************************************************************
bool CIRWifi::RemoveSensorAtExpression(int iIndexExpressionParam,int iIndexSensorParam)
{
  if (Expression.size() > iIndexExpressionParam)
  {
    if (Expression[iIndexExpressionParam]->Sensors.size() >  iIndexSensorParam)
    {
      CEventHandler *p_Sensor = Expression[iIndexExpressionParam]->Sensors[iIndexSensorParam];
      stExpressionItem *p_Item = Expression[iIndexExpressionParam];

      // remove cause operators
      if (Expression[iIndexExpressionParam]->Sensors.size() <= 2)
      {
        Expression[iIndexExpressionParam]->CauseOperators.clear();
      }
      else
      {
        if (iIndexSensorParam == 0)
        {
          Expression[iIndexExpressionParam]->CauseOperators.erase(Expression[iIndexExpressionParam]->CauseOperators.begin() + 0);  
        }
        else
        {
          Expression[iIndexExpressionParam]->CauseOperators.erase(Expression[iIndexExpressionParam]->CauseOperators.begin() + (iIndexSensorParam - 1));   
        }      
      }
      //remove sensor from expression
      Expression[iIndexExpressionParam]->Sensors.erase(Expression[iIndexExpressionParam]->Sensors.begin() + iIndexSensorParam);

      // memory disposal
      if (p_Sensor->GetEventType() == EVT_GPS_HANDLER)
      {
        delete (CEventGPSHandler *)p_Sensor;
      }
      else
      if (p_Sensor->GetEventType() == EVT_PRD_HANDLER)
      {
        delete (CEventPeriodHandler *)p_Sensor;
      }
      else
      if (p_Sensor->GetEventType() == EVT_WTR_HANDLER)
      {
        delete (CEventWeatherHandler *)p_Sensor;
      }

      
      if (Expression[iIndexExpressionParam]->Sensors.size() == 0)
      { 
        Expression.erase(Expression.begin() + iIndexExpressionParam);   
        delete p_Item;
      }
      SaveExpressions(MEM_INIT_EXPRESSION_POINTER);
      return  true;
    }
  }
  return false;
}

// ***********************************************************************************************************************************************************************************

void CIRWifi::HandleSpeechEvent(WStype_t type, uint8_t * payload, size_t length)
{
  switch(type) 
  {
    case WStype_DISCONNECTED:
      bSpeechServerConnected = false;    
      break;
    case WStype_CONNECTED: 
    {
      bSpeechServerConnected = true;      
    }
      break;
    case WStype_TEXT: 
    {
        Serial.printf("get text: %s\n", payload);
        // Example payloads

        // For Switch or Light device types
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html

        // For Light device type
        // Look at the light example in github
          
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload); 
        String deviceId = json ["deviceId"];     
        String strAction = json ["action"];
        if (deviceId == String(strSpeechDevID))
        {
          if(strAction == F("setPowerState")) //alexa
          {
            String strValue = json ["value"];
            if(strValue == "ON")
            {
              //RelayOn(0);
            }
            else
            {
              //RelayOff(0);
            }
          }
          else if (strAction == F("action.devices.commands.OnOff"))//google home
          {
            String strValue = json ["value"]["on"];
            if(strValue == "true")
            {
              //RelayOn(0);
            }
            else
            {
              //RelayOff(0);
            }
          }    
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("bin len: %u\n", length);
      break;
  }
}

// ***********************************************************************************************************************************************************************************
void CIRWifi::InformRelayStateOnSpeechServer(String deviceId, String value) 
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["deviceId"] = deviceId;
  root["action"] = "setPowerState";
  root["value"] = value;
  StreamString databuf;
  root.printTo(databuf);
  
  webSocket.sendTXT(databuf);
}
// ***********************************************************************************************************************************************************************************

void CIRWifi::SendIRCommand(int iIRCommandParam)
{
   Serial.printf("iFreq: %u\n", IRButtons[iIRCommandParam].uiFreq);
   Serial.printf("iSize: %u\n",IRButtons[iIRCommandParam].uiSize);
   Serial.println("RawData:");
   for (int i=0; i < IRButtons[iIRCommandParam].uiSize; i++)  Serial.printf("%u ",IRButtons[iIRCommandParam].p_uiRawData[i]);
   irsend.sendRaw((uint16_t *)IRButtons[iIRCommandParam].p_uiRawData, IRButtons[iIRCommandParam].uiSize, IRButtons[iIRCommandParam].uiFreq); //Note the approach used to automatically calculate the size of the array.
}
