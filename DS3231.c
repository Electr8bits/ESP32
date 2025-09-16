#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Domingo", "Lunes", "Martes","Miércoles", "Jueves", "Viernes", "Sábado"};

void setup() 
{
  Serial.begin(57600);
  Wire.begin();
  if (!rtc.begin()) 
  {
    Serial.println("No se encuentra el RTC");
    while (1) delay(10);
  }
  if (rtc.lostPower()) 
  {
    Serial.println("RTC sin hora, ajustar la fecha/hora");
    // rtc.adjust(DateTime(2025, 9, 16, 2, 44, 30));
  }
}

void loop() 
{
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  delay(1000);
}
