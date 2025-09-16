#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Dirección I2C y tamaño LCD

char daysOfTheWeek[7][12] = {
  "Domingo", "Lunes", "Martes", "Miércoles",
  "Jueves", "Viernes", "Sábado"
};

const int led2 = 2; // Pin del LED

void setup() 
{
  Serial.begin(57600);
  Wire.begin();
  pinMode(led2, OUTPUT); // LED como salida

  if (!rtc.begin()) {
    Serial.println("No se encuentra el RTC");
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC sin hora, ajustar la fecha/hora");
    // rtc.adjust(DateTime(2025, 9, 16, 2, 44, 30)); // Ajusta fecha/hora manual si es necesario
  }

  lcd.init();      // Inicializa LCD
  lcd.backlight(); // Enciende retroiluminación
}

void loop() 
{
  DateTime now = rtc.now();

  // Muestra en Serial
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

  // --- Condicional LED ---
  if (now.second() < 10) {     // Durante los primeros 10 segundos del minuto
    digitalWrite(led2, HIGH);  // Enciende LED
  } else {
    digitalWrite(led2, LOW);   // Apaga LED
  }

  // --- Mostrar solo la hora en LCD ---
  lcd.clear();
  lcd.setCursor(0, 0);
  if (now.hour() < 10) lcd.print('0');
  lcd.print(now.hour());
  lcd.print(':');
  if (now.minute() < 10) lcd.print('0');
  lcd.print(now.minute());
  lcd.print(':');
  if (now.second() < 10) lcd.print('0');
  lcd.print(now.second());

  delay(1000);
}
