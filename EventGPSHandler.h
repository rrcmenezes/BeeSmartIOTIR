#ifndef _EVENTGPSHANDLER_H_
#define _EVENTGPSHANDLER_H_

#include "defines.h"
#include <vector>
#include "EventHandler.h"
#include "IRWifi.h"


class CEventGPSHandler: public CEventHandler
{
  CIRWifi *p_IR;
  std::vector<ClientPosition> ClientPositions; 
  byte byKmThreshold;
  private: 
    double deg2rad(double deg) { return (deg * PI / 180.0); };
  public:
  CEventGPSHandler(CIRWifi *p_IRParam,unsigned long ulMinutesParam,byte byThresholdParam);                             // constructor
  ~CEventGPSHandler();                           // destuctor
    int GetEventType() { return EVT_GPS_HANDLER; };
    void SetClientPosition(String strNameParam,float fLatParam,float fLongParam);
    bool Update();
    void CheckConditions();
    void Save(int *pi_FilePointerPatam);
    String PrintExpression();
};



#endif 
