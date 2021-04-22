#ifndef _EVENTPERIODHANDLER_H_
#define _EVENTPERIODHANDLER_H_

#include "defines.h"
#include <vector>
#include "EventHandler.h"



class CEventPeriodHandler: public CEventHandler
{
  byte byDaysOfWeek;
  int iHourMinuteIni;
  int iHourMinuteEnd;
  int iLastHourMinuteCheck;
  public:
  CEventPeriodHandler(unsigned long ulMinutesParam,byte bDaysWeekParam, int iBegHourMinuteParam, int iEndHourMinuteParam);                             // constructor
  ~CEventPeriodHandler();                           // destuctor
    int GetEventType() { return EVT_PRD_HANDLER; };
    void CheckConditions();
    void Save(int *pi_FilePointerPatam);
    void Load(int *pi_FilePointerPatam);
    String PrintExpression();
};



#endif 
