#ifndef _EVENTHANDLER_H_
#define _EVENTHANDLER_H_
#include <arduino.h>

class CEventHandler
{
  unsigned long ulLstMillis;
  unsigned long ulCurrMillis;
  
  protected:
  unsigned long ulMinutes;
  bool bConditionsOK;
  CEventHandler(unsigned long ulMinutesParam);
  public:
  virtual void Save(int *pi_FilePointerPatam);
  virtual void Load(int *pi_FilePointerPatam);
  void Update();
  virtual int GetEventType() = 0;
  virtual void CheckConditions() = 0;
  virtual String PrintExpression() = 0;
  bool ConditionsOK() { return bConditionsOK; };
  void SetInterval(unsigned long ulMinutesParam) { ulMinutes = ulMinutesParam; ulLstMillis = millis();} ;
};

#endif
