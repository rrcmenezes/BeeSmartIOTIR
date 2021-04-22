#include "EventHandler.h"
#include "defines.h"


CEventHandler::CEventHandler(unsigned long ulMinutesParam) 
{ 
  ulMinutes = ulMinutesParam; 
  ulLstMillis = ulMinutesParam * -1  * 60 * 1000;
}

/*********************************************************************************************************************************/
void CEventHandler::Update()
{
  ulCurrMillis = millis();
  
  if ((ulCurrMillis - ulLstMillis) >= (ulMinutes * 60 * 1000))
  {
    ulLstMillis = ulCurrMillis;
    CheckConditions();
  }
}
/*****************************************************************************************************************************************************************/
void CEventHandler::Save(int *pi_FilePointerPatam)
{
  EEPROM_writeAnything(*pi_FilePointerPatam,(unsigned long)ulMinutes);
  *pi_FilePointerPatam += sizeof(unsigned long);
}
/*****************************************************************************************************************************************************************/

void CEventHandler::Load(int *pi_FilePointerPatam)
{
  EEPROM_writeAnything(*pi_FilePointerPatam,(unsigned long)ulMinutes);
  *pi_FilePointerPatam += sizeof(unsigned long);
}
/*****************************************************************************************************************************************************************/
