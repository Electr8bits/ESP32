#include <WiFi.h>

const char* ssid="MAJU 2.4G";
const char* password="luhanaevaluna";

void setup()
{
  Serial.begin(115200);
  Serial.println("\nConectando a WiFi...");
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED)
  {Serial.print(".");
    delay(500);}
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  
}
