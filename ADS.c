#include "ADS1X15.h"

ADS1115 ADS(0x48);  // Dirección 12C

void setup() 
{
  Serial.begin(115200);
  ADS.begin();
}

void loop() 
{
  //int16_t porque toma los valores -32768 a +32767
  int16_t val0 = ADS.readADC(0);  // Canal A0
  int16_t val1 = ADS.readADC(1);  // Canal A1
  int16_t val2 = ADS.readADC(2);  // Canal A2
  int16_t val3 = ADS.readADC(3);  // Canal A3

  //Mostrar en el monitor serial
  Serial.print("A0: "); 
  Serial.print(val0);

  delay(1000);  // 1s
}
