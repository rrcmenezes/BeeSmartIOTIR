#include "EventWeatherHandler.h"
#include "EventHandler.h"

#include <arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

CEventWeatherHandler::CEventWeatherHandler(CIRWifi *p_IRParam,unsigned long ulMinutesParam,float fMinTempParam, float fMaxTempParam,String strWeatherParam)
                         :CEventHandler(ulMinutesParam)
{
  fMaxTemperature = fMaxTempParam;
  fMinTemperature = fMinTempParam;
  p_IR = p_IRParam;
  strWeatherConfig = strWeatherParam;
  strLocationObserv = "";

  RequestWeatherConditions(true); 
  
 // DEBUG_PRINTLN(" minutestoupdate: "+ String(ulMinutesParam));
 // DEBUG_PRINTLN(" MaxTemperature: " + String(fMaxTemperature));
 // DEBUG_PRINTLN(" MinTemperature: " + String(fMinTemperature));
 // DEBUG_PRINTLN(" WeatherConfig: " + strWeatherConfig);
 // DEBUG_PRINTLN(" OutsideWeather: " + strOutsideWeather);
 // DEBUG_PRINTLN(" OutsideTemperature: " + String(dbOutsideTemperature));
}
//-------------------------------------------------------------------------------------------------------------------------------

CEventWeatherHandler::~CEventWeatherHandler()
{
  
}
//-------------------------------------------------------------------------------------------------------------------------------

String CEventWeatherHandler::PrintExpression()
{
  String strExpression;

  if (strWeatherConfig.toInt() == 0)
  {
    strExpression = "temperatura atual de " + String(dbOutsideTemperature) + " entre ";
    strExpression += String(fMinTemperature);
    strExpression += " e ";
    strExpression += String(fMaxTemperature);
  
  }

  if (strWeatherConfig.toInt() != 0 && fMaxTemperature == 0 && fMinTemperature == 0)
  {
    strExpression = "clima atual " + strOutsideWeather + " com estado ( ";

    if ( strWeatherConfig.substring(0,1) == "1") strExpression += String("limpo ");
    if ( strWeatherConfig.substring(1,2) == "1") strExpression += String("parcialmente nublado ");
    if ( strWeatherConfig.substring(2,3) == "1") strExpression += String("nublado ");
    
    strExpression += String(")");
  }

  if (strWeatherConfig.toInt() != 0 && (fMaxTemperature != 0 || fMinTemperature != 0))
  {
    strExpression = "temperatura atual de " + String(dbOutsideTemperature) + " entre ";
    strExpression += String(fMinTemperature);
    strExpression += " e ";
    strExpression += String(fMaxTemperature);
    strExpression += " e ";
    strExpression += "clima atual " + strOutsideWeather + " com estado ( ";

    if ( strWeatherConfig.substring(0,1) == "1") strExpression += String("limpo ");
    if ( strWeatherConfig.substring(1,2) == "1") strExpression += String("parcialmente nublado ");
    if ( strWeatherConfig.substring(2,3) == "1") strExpression += String("nublado ");
    
    strExpression += String(")");
  }

  return strExpression;
}

//-------------------------------------------------------------------------------------------------------------------------------

void CEventWeatherHandler::SetModule(CIRWifi *p_IRParam)
{
  p_IR = p_IRParam;
}
//-------------------------------------------------------------------------------------------------------------------------------

void CEventWeatherHandler::CheckConditions()
{
  bool bCanBeActive = true;
  
  if (strLocationObserv != "")
  {
    RequestWeatherConditions(false);            
  }
  else
  {
    RequestWeatherConditions(true); 
  }

  if ((fMaxTemperature != 0.0f) || (fMinTemperature != 0.0f))
  {
    if ((dbOutsideTemperature <= fMaxTemperature) && (dbOutsideTemperature >= fMinTemperature))
    {
      bCanBeActive = true;
    }
    else
    {
      bCanBeActive = false;
    }
  }

  if (bCanBeActive && (strWeatherConfig.toInt() != 0))
  {
    if (strOutsideWeather == "Clear" && strWeatherConfig.substring(0,1) == "0")
    {
      bCanBeActive = false;
    }
    else
    {
      bCanBeActive =true;
    }
    if (strOutsideWeather == "Partly Cloudy" && strWeatherConfig.substring(1,2) == "0")
    {
      bCanBeActive = false;
    }
    else
    {
      bCanBeActive =true;
    }
    if (strOutsideWeather == "Cloudy" && strWeatherConfig.substring(2,3) == "0")
    {
      bCanBeActive = false;
    }
    else
    {
      bCanBeActive =true;
    }
  }

  if (bCanBeActive)
  { 
      bConditionsOK = true;
  }
  else
  {
      bConditionsOK = false;
  }

  //DEBUG_PRINTLN("=============== Weather Event Handler ================:");
  //DEBUG_PRINTLN(" LocatlnionObserv: "+ strLocationObserv);
  //DEBUG_PRINTLN(" MaxTemperature: " + String(fMaxTemperature));
  //DEBUG_PRINTLN(" MinTemperature: " + String(fMinTemperature));
  //DEBUG_PRINTLN(" WeatherConfig: " + strWeatherConfig);
  //DEBUG_PRINTLN(" OutsideWeather: " + strOutsideWeather);
  //DEBUG_PRINTLN(" OutsideTemperature: " + String(dbOutsideTemperature));
}

/***************************************************************************************************** */
void CEventWeatherHandler::Save(int *pi_FilePointerPatam)
{ 
  CEventHandler::Save(pi_FilePointerPatam);
  for (int i = 0; i < strLocationObserv.length(); i++)
  {
    EEPROM.write(*pi_FilePointerPatam + i, strLocationObserv[i]);
  }
  *pi_FilePointerPatam += LOCAYTION_OBSERV_SIZE;
  EEPROM_writeAnything(*pi_FilePointerPatam,(float)fMinTemperature);  // horario fim
  *pi_FilePointerPatam += sizeof(float);
  EEPROM_writeAnything(*pi_FilePointerPatam,(float)fMaxTemperature);  // horario fim
  *pi_FilePointerPatam += sizeof(float);
  
  
  EEPROM.commit();  
}
/***************************************************************************************************** */
void CEventWeatherHandler::Load(int *pi_FilePointerPatam)
{ 
  CEventHandler::Load(pi_FilePointerPatam);
  strLocationObserv = "";
  char chChar;
  for (int i = *pi_FilePointerPatam; i < *pi_FilePointerPatam + LOCAYTION_OBSERV_SIZE; i++)
  {
    chChar = char(EEPROM.read(i));
    if (isGraph(chChar))
    {
      strLocationObserv += chChar;
    }
    else
    {
      break;
    }
  }
  *pi_FilePointerPatam += LOCAYTION_OBSERV_SIZE;
  
 // DEBUG_PRINT("location Observ: ");
 // DEBUG_PRINTLN(strLocationObserv);
  EEPROM_readAnything(*pi_FilePointerPatam,fMinTemperature);  // temp min
  *pi_FilePointerPatam += sizeof(float);
  EEPROM_readAnything(*pi_FilePointerPatam,fMaxTemperature);  // temp max
  *pi_FilePointerPatam += sizeof(float);
}

/***************************************************************************************************** */
bool CEventWeatherHandler::RequestWeatherConditions(bool bForceUpdateLocalObserv)
{
  String strHost;
  String strGeoReq = "/geolookup/q/";
  String strKey;
  String strResponse;
  
  strHost   += p_IR->GetWeatherAPIURL();
  strKey = p_IR->GetWeatherAPIKey();
     
  DynamicJsonBuffer jsonBuffer;
  WiFiClient client;
  if (p_IR)
  {
    
    if (p_IR->IsGeoKnown())
    {    
      if (bForceUpdateLocalObserv)
      {
     //   DEBUG_PRINTLN("connecting to server (to local observation) " + strHost);
        if (client.connect(strHost.c_str(), 80)) 
        {
          String strFeatures = "geolookup";
       //   DEBUG_PRINTLN("weather geolookup webservice Connected!");
    
          // Make a HTTP request:
        //  DEBUG_PRINTLN("http request (geolookup):");
         // DEBUG_PRINTLN("GET /api/" + strKey + "/" + strFeatures + "/q/" +  String(p_IR->GetLatitudePosition(),6) + "," + String(p_IR->GetLongitudePosition(),6) + ".json HTTP/1.1\r\n" +
       //   "Host: " + strHost + "\r\n" +
       //   "Content-Type: application/json\r\n" +
       //   "Connection: close\r\n\r\n");
                
          client.print("GET /api/" + strKey + "/" + strFeatures + "/q/" +  String(p_IR->GetLatitudePosition(),6) + "," + String(p_IR->GetLongitudePosition(),6) + ".json HTTP/1.1\r\n" +
          "Host: " + strHost + "\r\n" +
          "Content-Type: application/json\r\n" +
          "Connection: close\r\n\r\n");

          delay(1000);

          strResponse = "";
         
          bool bStartCapture = false; 
          
          
          while (client.connected())
          {
            if ( client.available())
            {
              char c = client.read();
              if(c == '{')
                bStartCapture=true;
        
              if(bStartCapture)
              {
                strResponse += c;
              }
            }
          }
          
          
          JsonObject& root = jsonBuffer.parseObject(strResponse);
                
          if (root.success())
          {
            const char* nome = root["location"]["l"];
            strLocationObserv = nome;
           // DEBUG_PRINTLN("LocationObserv: " + strLocationObserv);
          }
          else
          {
           // DEBUG_PRINTLN("location root json failed");
           // DEBUG_PRINTLN("Server Response:");
           // DEBUG_PRINTLN(strResponse);         
            client.stop();
            return false;              
          }
          
          
          client.stop();
        }
        else
        {
         // DEBUG_PRINTLN("failed to connect server (local observ)");
          return false;
        }
      }
    
    
     // DEBUG_PRINTLN("Requesting weather (conditions)");
      if (client.connect(strHost.c_str(), 80)) 
      {
        String strFeatures = "conditions";
      //  DEBUG_PRINTLN("weather conditions webservice Connected!");
        
        // Make a HTTP request:
     //   DEBUG_PRINTLN("GET /api/" + strKey + "/" + strFeatures + strLocationObserv + ".json HTTP/1.1\r\n" +
     //   "Host: " + strHost + "\r\n" +
     //   "Connection: close\r\n\r\n");
        
        client.print("GET /api/" + strKey + "/" + strFeatures + strLocationObserv + ".json HTTP/1.1\r\n" +
        "Host: " + strHost + "\r\n" +
        "Connection: close\r\n\r\n");

          
          delay(1000);
          
          
          bool bStartCapture = false;
          strResponse = "";
     
          while (client.connected())
          {
            if ( client.available())
            {
              char c = client.read();
              if(c == '{')
                bStartCapture=true;
        
              if(bStartCapture)
              {
                strResponse += c;
              }
            }
          }
          
            
        JsonObject& root = jsonBuffer.parseObject(strResponse);
    
        if (root.success())
        {
          dbOutsideTemperature = root["current_observation"]["temp_c"];
          const char* weather = root["current_observation"]["weather"];
          strOutsideWeather = weather;
        }
        else
        {
          //DEBUG_PRINTLN("temperature/climate json failed");
          //DEBUG_PRINTLN("Server Response:");
         // DEBUG_PRINTLN(strResponse);              
          client.stop();
          return false;
        }
        
        client.stop();
      }
      else
      {
        //DEBUG_PRINTLN("Failed to request weather (conditions)");
        return false;
      }
    return true;
    }
  }
  return false;
}
/***************************************************************************************************************************************************/
