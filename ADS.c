#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;  

const float Fc = 0.000125;  // V/bit con GAIN_ONE (±4.096 V)

// Variables para cada canal
float volt0, volt1, volt2, volt3;

void setup(void) 
{
  Serial.begin(115200);
  Serial.println("Iniciando ADS1115...");

  if (!ads.begin()) {
    Serial.println("No se encontró el ADS1115, revisa las conexiones!");
    while (1);
  }

  ads.setGain(GAIN_ONE);  // ±4.096 V
}

void loop(void) 
{
  for (int ch = 0; ch < 4; ch++) 
  {
    int16_t lectura_adc = ads.readADC_SingleEnded(ch);
    float voltage = lectura_adc * Fc;

    if (ch == 0) volt0 = voltage;
    else if (ch == 1) volt1 = voltage;
    else if (ch == 2) volt2 = voltage;
    else if (ch == 3) volt3 = voltage;
  }

  // Imprime las variables
  Serial.print("A0: "); Serial.print(volt0, 2); Serial.print(" V  ");
  Serial.print("A1: "); Serial.print(volt1, 2); Serial.print(" V  ");
  Serial.print("A2: "); Serial.print(volt2, 2); Serial.print(" V  ");
  Serial.print("A3: "); Serial.print(volt3, 2); Serial.println(" V");

  delay(1000);
}
