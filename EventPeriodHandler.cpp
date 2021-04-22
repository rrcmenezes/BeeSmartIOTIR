#include "EventPeriodHandler.h"
#include "EventHandler.h"
#include "defines.h"

#include <arduino.h>
#include <TimeLib.h>

CEventPeriodHandler::CEventPeriodHandler(unsigned long ulMinutesParam,byte bDaysWeekParam, int iBegHourMinuteParam, int iEndHourMinuteParam)
                    :CEventHandler(ulMinutesParam)
{
  iHourMinuteIni = iBegHourMinuteParam;
  iHourMinuteEnd = iEndHourMinuteParam;
  byDaysOfWeek = bDaysWeekParam;

//  DEBUG_PRINTLN(" minutestoupdate: "+ String(ulMinutesParam));
//  DEBUG_PRINTLN(" iHourMinuteIni: " + String(iHourMinuteIni));
//  DEBUG_PRINTLN(" iHourMinuteEnd: " + String(iHourMinuteEnd));
//  DEBUG_PRINT(" bDaysWeekParam: ");
//  DEBUG_PRINTFORMATTED(bDaysWeekParam,BIN);

}
//-------------------------------------------------------------------------------------------------------------------------------

CEventPeriodHandler::~CEventPeriodHandler()
{
  
}
//-------------------------------------------------------------------------------------------------------------------------------

String CEventPeriodHandler::PrintExpression()
{
  String strExpression;

  strExpression = "hora atual entre ";
  strExpression += String(iHourMinuteIni);
  strExpression += " e ";
  strExpression += String(iHourMinuteEnd);
  if (byDaysOfWeek)
  {
    strExpression += " e dia da semana entre ( ";
    
    if (( byDaysOfWeek & DOMINGO) == DOMINGO) strExpression += String("domingo ");
    if (( byDaysOfWeek & SEGUNDA) == SEGUNDA) strExpression += String("segunda ");
    if (( byDaysOfWeek & TERCA) == TERCA) strExpression += String("terca ");
    if (( byDaysOfWeek & QUARTA) == QUARTA) strExpression += String("quarta ");
    if (( byDaysOfWeek & QUINTA) == QUINTA) strExpression += String("quinta ");
    if (( byDaysOfWeek & SEXTA) == SEXTA) strExpression += String("sexta ");
    if (( byDaysOfWeek & SABADO) == SABADO) strExpression += String("sabado ");
    
    strExpression += String(")");
  }
  
  return strExpression;
}

//-------------------------------------------------------------------------------------------------------------------------------

void CEventPeriodHandler::CheckConditions()
{
  int i;
  int iHourMinute;
 
  iHourMinute = (hour() * 100) + minute();

  byte byDay = 1 << (7 - (byte)weekday());

    
//    if ((iHourMinute <= iHoraMinutos) && (iHourMinute > iLastHourMinuteCheck) && ((byDaysOfWeek & byDay) == byDay))
//    {
//      bConditionsOK = true;
//    }
//  
  bConditionsOK = false;
  if ((iHourMinute <= iHourMinuteEnd) && (iHourMinute >= iHourMinuteIni) && ((byDaysOfWeek & byDay) == byDay))
  {
      bConditionsOK = true;
  }
}

/***************************************************************************************************** */
void CEventPeriodHandler::Save(int *pi_FilePointerPatam)
{ 
  CEventHandler::Save(pi_FilePointerPatam);
       
  EEPROM_writeAnything(*pi_FilePointerPatam,(byte)byDaysOfWeek);  // dias da semana
  *pi_FilePointerPatam += sizeof(byte);

  EEPROM_writeAnything(*pi_FilePointerPatam,(int)iHourMinuteIni);  // horario inicio
  *pi_FilePointerPatam += sizeof(int);

  EEPROM_writeAnything(*pi_FilePointerPatam,(int)iHourMinuteEnd);  // horario fim
  *pi_FilePointerPatam += sizeof(int);
}
/***************************************************************************************************** */
void CEventPeriodHandler::Load(int *pi_FilePointerPatam)
{ 
  CEventHandler::Load(pi_FilePointerPatam);

  EEPROM_readAnything(*pi_FilePointerPatam,byDaysOfWeek);  // dias da semana
  *pi_FilePointerPatam += sizeof(byte);

  EEPROM_readAnything(*pi_FilePointerPatam,iHourMinuteIni);  // horario trigger
  *pi_FilePointerPatam += sizeof(int);

  EEPROM_readAnything(*pi_FilePointerPatam,iHourMinuteEnd);  // horario trigger
  *pi_FilePointerPatam += sizeof(int);
  
  
}

/***************************************************************************************************** */

/***************************************************************************************************************************************************/
