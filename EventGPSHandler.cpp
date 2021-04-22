#include "EventGPSHandler.h"
#include "EventHandler.h"

#include <arduino.h>

CEventGPSHandler::CEventGPSHandler(CIRWifi *p_IRParam,unsigned long ulMinutesParam,byte byThresholdParam)
                 :CEventHandler(ulMinutesParam)
{
  byKmThreshold = byThresholdParam;
  p_IR = p_IRParam;
}
//-------------------------------------------------------------------------------------------------------------------------------

CEventGPSHandler::~CEventGPSHandler()
{
  
}
//-------------------------------------------------------------------------------------------------------------------------------

String CEventGPSHandler::PrintExpression() 
{ 
  String strExpression;

  strExpression = "USUARIO(S) ";
  for (int i = ClientPositions.size(); i > 0; i--)
  {
    strExpression += ClientPositions[i].strName;
    if (i != 1)
    {
      strExpression += " OU ";
    }
  }
  strExpression = " ABAIXO DOS " + String(byKmThreshold) + " KM DE DISTANCIA DO MODULO";
  
};
//-------------------------------------------------------------------------------------------------------------------------------

void CEventGPSHandler::CheckConditions()
{
  int R = 6371; // Radius of the earth


  for (int i = 0; i < ClientPositions.size(); i++)
  {
    double latDistance = deg2rad(ClientPositions[i].vPos.fLat - p_IR->GetLatitudePosition());
    double lonDistance = deg2rad(ClientPositions[i].vPos.fLong - p_IR->GetLongitudePosition());
    double a = sin(latDistance / 2) * sin(latDistance / 2)
             + cos(deg2rad(p_IR->GetLatitudePosition())) * cos(deg2rad(ClientPositions[i].vPos.fLat))
             * sin(lonDistance / 2) * sin(lonDistance / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double distance = R * c * 1000; // convert to meters

    distance = sqrt(pow(distance, 2));

//    DEBUG_PRINTLN("GPS DISTANCE (METERS): " + String(distance));
    
    if (distance < (byKmThreshold * 1000))
    {
      bConditionsOK = true;
      return;
    }
  }
  bConditionsOK = false;
  
}

/***************************************************************************************************** */

void CEventGPSHandler::Save(int *pi_FilePointerPatam)
{ 
  CEventHandler::Save(pi_FilePointerPatam);
}

/***************************************************************************************************** */

void CEventGPSHandler::SetClientPosition(String strNameParam,float fLatParam,float fLongParam)
{
  for (int i = 0; i < ClientPositions.size(); i++)
  {
    if (ClientPositions[i].strName == strNameParam)
    {
      ClientPositions[i].vPos.fLat = fLatParam;
      ClientPositions[i].vPos.fLat = fLongParam;
      return;
    }
  }

  ClientPosition cPos;

  cPos.vPos.fLat = fLatParam;
  cPos.vPos.fLong = fLongParam;
  cPos.strName = strNameParam;
  ClientPositions.push_back(cPos);
  
  
}

/***************************************************************************************************** */
