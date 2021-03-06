#include "IRWifi.h"


//prototypes
void mqtt_callback(char* topic, byte* payload, unsigned int length);



CIRWifi Modulo;


//int freeRam() 
//{
///  extern int __heap_start, *__brkval; 
///  int v; 
//  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
//}

/*------------------------------------------------------------------------------------------------------------ */
void setup()
{
  delay(2000);
  Serial.begin(115200);
  delay(4000);  
  Modulo.Init();
  Modulo.MQTTClient.setCallback(mqtt_callback);
  // event handler
  Modulo.webSocket.onEvent(webSocketEvent);

}


/* -----------------------------------------------------------------------------------------------------------*/
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) 
{
  Modulo.HandleSpeechEvent(type,payload,length);
  
}

/*------------------------------------------------------------------------------------------------------------ */

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  String msg;
  int iIndexExpression;
  int iIndexSensor;
  
  //obtem a string do payload recebido
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
  }
  msg.toLowerCase();
  
  Serial.println(F("Topic:"));
  Serial.println(topic);
  Serial.println(msg);
  
  //toma ação dependendo da string recebida:

  // toggle ir TV on off
  if (msg.substring(0, 3) == "ir0")
  {
    Modulo.SendIRCommand(IR_TV_ONOFF);
  }

  if (msg.substring(0, 3) == "cef")
{
  iIndexExpression = msg.substring(3, 4).toInt();
  Modulo.ChangeEffectOperatorExpression(iIndexExpression,msg.substring(4, 5)[0]);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CCCEA   -- mudar operador de efeito
// CCC    -- codigo do comando cef
// E      -- indice da expressao
// A      -- 1 - TRUE_ENABLE BOTH 2 - TRUE_DISABLE BOTH 3 - TRUE_ENABLE_ELSE BOTH 4 - TRUE_DISABLE_ELSE BOTH
//           5 - TRUE_ENABLE_R1   6 - TRUE_DISABLE_R1   7 - TRUE_ENABLE_ELSE R1   8 - TRUE_DISABLE_ELSE R1
//           9 - TRUE_ENABLE_R1   10 - TRUE_DISABLE_R1 11 - TRUE_ENABLE_ELSE R1  12 - TRUE_DISABLE_ELSE R1

if (msg.substring(0, 3) == "cef")
{
  iIndexExpression = msg.substring(3, 4).toInt();
  Modulo.ChangeEffectOperatorExpression(iIndexExpression,msg.substring(4, 5)[0]);
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  // Deleta expressao. remove uma expressao existente e todos os seus handlers e operadores  
  // CCCESSSSSSSIIIIFFFF
  // CCC     -- Codigo do comando (dex)
  // E       -- indice da expressao
  if (msg.substring(0, 3) == "dex")
  {
    int iIndexExpression = msg.substring(3, 4).toInt();
    
    //Modulo.RemoveExpression(iIndexExpression);
  }

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  // Deleta sensor na expressao. remove somente um sensor, se nao tiver mais sensores,remove toda a expressao  
  // CCCES
  // CCC     -- Codigo do comando (dex)
  // E       -- indice da expressao
  // S       -- indice do sensor
  if (msg.substring(0, 3) == "dse")
  {
    iIndexExpression = msg.substring(3, 4).toInt();
    iIndexSensor     = msg.substring(4, 5).toInt();
  
    Modulo.RemoveSensorAtExpression(iIndexExpression,iIndexSensor);
  }

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  // adiciona Periodo handler no modulo. verifica se o modulo encontra-se dentro do periodo dia_semana/hora/min inicial dia_semana/hora/min final em relacao a hora local  
  // CCCESSSSSSSIIIIFFFF
  // CCC     -- Codigo do comando
  // E       -- indice da expressao
  // SSSSSSS -- dias da semana que devem ser considerados 7 dias para cada digito, (comecando por domingo) por exemplo, 1000010 deve ser considerado domingo e sexta
  // IIII    -- hora/min inicial do periodo em formato HHMM
  // FFFF    -- hora/min final do periodo em formato HHMM
 
  if (msg.substring(0, 3) == "prd")
  {
    
    Modulo.AddEventHandler(EVT_PRD_HANDLER,msg);

  }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

   // adiciona gps ao modulo  
  // CCCEKKKUUUUUUUUUUAAAAAAAAOOOOOOOO
  // CCC     -- Codigo do comando (gps)
  // E       -- indice da expressao
  // KKK     -- Raio de definicao parra considerar o moddulo perto ou longe dos usuarios (kilometros)
  // UUUUUUUUUU  -- Usuário que enviou o sensor
  // AAAAAAAA    -- user latitude position
  // OOOOOOOO    -- user longitude position
 
  if (msg.substring(0, 3) == "gps") // 
  {
    Modulo.AddEventHandler(EVT_GPS_HANDLER,msg);  
  }

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  
  // eru - toggle expressions  
  // CCCX
  // CCC         -- Codigo do comando (e)
  
  if (msg.substring(0, 3) == "tru") // 
  {
    int iValue = msg.substring(3, 4).toInt();
    if (iValue == 0)
    {
      Modulo.bEnableExpressions = false;
    }
    else
    {
      Modulo.bEnableExpressions = true;      
    }
  }
  
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  
  // atualiza posicao do usuario nas regras do modulo  
  // CCCUUUUUUUUUUAAAAAAAAOOOOOOOO
  // CCC         -- Codigo do comando (uup)
  // UUUUUUUUUU  -- Usuário que enviou a posicao
  // AAAAAAAA    -- latitude position
  // OOOOOOOO    -- longitude position
  
  if (msg.substring(0, 3) == "uup") // 
  {
    String strUser;

    strUser = msg.substring(3, 13);
    
    Modulo.SetUserPosition(strUser,msg.substring(13, 21).toFloat(),msg.substring(21, 29).toFloat());
  }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // adiciona clima handler no modulo. verifi ca se o modulo encontra-se dentro do periodo dia_semana/hora/min inicial dia_semana/hora/min final em relacao a hora locale randomiza o estado do relé  
  // CCCEMMMXXXYYYWWWW
  // CCC     -- Codigo do comando (wtr)
  // E       -- Indice da expressao
  // MMM     -- minutos para checar webserviceque retorna dados da temperatura
  // XXX     -- temperatura minima do intervalo
  // YYY     -- temperatura maxima do intervalo
  // WWWW    -- clima codificado (clear, partly cloudy, cloudy). 1100 por exemplo épra ativar sempre com clear e partly cloudy
  
  if (msg.substring(0, 3) == "wtr")
  {   
    if (!Modulo.AddEventHandler(EVT_WTR_HANDLER,msg))
    {
   //   DEBUG_PRINTLN("cannot insert weather");
    //  DEBUG_PRINT("weatherAPI: ");
   //   DEBUG_PRINTLN(Modulo.GetWeatherAPIKey());
   //   DEBUG_PRINTLN("IsGeoKnown: " + String(Modulo.IsGeoKnown()));
    }
  }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  //verifica se deve limpar a memoria do dispositivo
  if (msg.equals("c"))
  {
    Modulo.Format();
  //  DEBUG_PRINTLN("Modulo Formatado.Rebotando em 5 segundos...");
    delay(5);
    Modulo.Reset();
  }
  //Serial.print(F("Free Memory: "));
  //int el = freeRam();
  //Serial.println(el);
}

/******************************************************************************************************************************* */

void loop()
{
  Modulo.Update();
  
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
