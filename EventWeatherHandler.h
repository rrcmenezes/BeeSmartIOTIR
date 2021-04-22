#ifndef _EVENTWEATHERHANDLER_H_
#define _EVENTWEATHERHANDLER_H_

#include "defines.h"
#include <vector>
#include "IRWifi.h"
#include "EventHandler.h"

#define LOCAYTION_OBSERV_SIZE       32
#define INTERVAL_WEATHER (60000 * 2)

#define CLEAR   (byte)1
#define PARTLY_CLOUDY (byte)2
#define CLOUDY     (byte)4
#define RAINY     (byte)8
#define gg     (byte)16
#define kk      (byte)32
#define oo     (byte)64

class CEventWeatherHandler: public CEventHandler
{
  CIRWifi *p_IR;
  String strLocationObserv;
  float fMaxTemperature;
  float fMinTemperature;
  String strWeatherConfig;
  String strOutsideWeather;
  double dbOutsideTemperature;
  bool RequestWeatherConditions(bool bForceUpdateLocalObserv); 
  public:
  CEventWeatherHandler(CIRWifi *p_IRParam,unsigned long ulMinutesParam,float fMinTempParam, float fMaxTempParam,String strWeatherParam);                             // constructor
  ~CEventWeatherHandler();                           // destuctor
    int GetEventType() { return EVT_WTR_HANDLER; };
    bool Update();
    void CheckConditions();
    void Save(int *pi_FilePointerPatam);
    void Load(int *pi_FilePointerPatam);
    void SetModule(CIRWifi *p_IRParam);
    String PrintExpression();
};



#endif 
