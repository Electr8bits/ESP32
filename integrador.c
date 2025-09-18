#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_ADS1X15.h>

// --- WiFi ---
const char* ssid = "MAJU 2.4G";
const char* password = "luhanaevaluna";

// --- Pines de cada salida ---
const int pinLuz    = 25;
const int pinToma1  = 26;
const int pinToma2  = 27;
const int led2      = 2;

// --- Variables de estado ---
bool estadoLuz = false;
bool estadoToma1 = false;
bool estadoToma2 = false;

// --- RTC ---
RTC_DS3231 rtc;
String horaRTC = "";

// --- ADS1115 ---
Adafruit_ADS1115 ads;
const float Fc = 0.000125;  // V/bit con GAIN_ONE (±4.096 V)
float voltCorriente1 = 0, voltCorriente2 = 0, voltVoltaje = 0;
float corrienteTotal = 0;

AsyncWebServer server(80);

void setup() 
{
  pinMode(pinLuz, OUTPUT);   digitalWrite(pinLuz, HIGH);
  pinMode(pinToma1, OUTPUT); digitalWrite(pinToma1, HIGH);
  pinMode(pinToma2, OUTPUT); digitalWrite(pinToma2, HIGH);
  pinMode(led2, OUTPUT);     digitalWrite(led2, LOW);

  Serial.begin(115200);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {Serial.print("."); delay(500);}
  Serial.println("\nConectado!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // RTC
  Wire.begin();
  if (!rtc.begin()) { Serial.println("No se encuentra el RTC"); while (1) delay(10); }
  if (rtc.lostPower()) { Serial.println("RTC sin hora, ajustar la fecha/hora"); }

  // ADS1115
  Serial.println("Iniciando ADS1115...");
  if (!ads.begin()) { Serial.println("No se encontró el ADS1115!"); while(1); }
  ads.setGain(GAIN_ONE);

  // Página principal
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;text-align:center;background:#f4f4f4;}h1{color:#333;}button{padding:10px 20px;margin:10px;font-size:16px;border:none;border-radius:5px;} .on{background:#4CAF50;color:white;} .off{background:#f44336;color:white;}</style>";
    html += "<script>";
    html += "function toggle(dev){";
    html += "fetch('/toggle?dev='+dev).then(r=>r.text()).then(t=>{";
    html += "document.getElementById(dev).innerHTML=t;";
    html += "document.getElementById(dev).className=(t.includes('ON')?'on':'off');});}";
    html += "setInterval(()=>{";
    html += "fetch('/hora').then(r=>r.text()).then(t=>{document.getElementById('hora').innerHTML=t;});";
    html += "fetch('/medidas').then(r=>r.text()).then(t=>{document.getElementById('medidas').innerHTML=t;});},1000);";
    html += "</script></head><body>";
    html += "<h1>Panel de Control</h1>";
    html += "<h2>Hora RTC: <span id='hora'>--:--:--</span></h2>";
    html += "<div id='medidas'>Cargando medidas...</div><br>";

    html += "<button id='luz' class='" + String(estadoLuz ? "on":"off") + "' onclick=\"toggle('luz')\">Luz " + String(estadoLuz ? "ON":"OFF") + "</button><br>";
    html += "<button id='toma1' class='" + String(estadoToma1 ? "on":"off") + "' onclick=\"toggle('toma1')\">Tomacorriente 1 " + String(estadoToma1 ? "ON":"OFF") + "</button><br>";
    html += "<button id='toma2' class='" + String(estadoToma2 ? "on":"off") + "' onclick=\"toggle('toma2')\">Tomacorriente 2 " + String(estadoToma2 ? "ON":"OFF") + "</button><br>";

    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Toggle salidas
  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    String respuesta = "Error";
    if (request->hasParam("dev")) {
      String dev = request->getParam("dev")->value();
      if (dev == "luz") { estadoLuz = !estadoLuz; digitalWrite(pinLuz, estadoLuz?LOW:HIGH); respuesta = "Luz "+String(estadoLuz?"ON":"OFF"); }
      else if (dev == "toma1") { estadoToma1 = !estadoToma1; digitalWrite(pinToma1, estadoToma1?LOW:HIGH); respuesta = "Tomacorriente 1 "+String(estadoToma1?"ON":"OFF"); }
      else if (dev == "toma2") { estadoToma2 = !estadoToma2; digitalWrite(pinToma2, estadoToma2?LOW:HIGH); respuesta = "Tomacorriente 2 "+String(estadoToma2?"ON":"OFF"); }
    }
    request->send(200, "text/plain", respuesta);
  });

  // Hora RTC
  server.on("/hora", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", horaRTC);
  });

  // Medidas ADS1115
  server.on("/medidas", HTTP_GET, [](AsyncWebServerRequest *request){
    String datos = "Corriente Toma1: " + String(voltCorriente1,2) + " V<br>";
    datos += "Corriente Toma2: " + String(voltCorriente2,2) + " V<br>";
    datos += "Corriente Total: " + String(corrienteTotal,2) + " V<br>";
    datos += "Voltaje: " + String(voltVoltaje,2) + " V<br>";
    request->send(200, "text/html", datos);
  });

  server.begin();
}

void loop() {
  // Hora
  DateTime now = rtc.now();
  horaRTC = String(now.hour()) + ":" + (now.minute()<10?"0":"") + String(now.minute()) + ":" + (now.second()<10?"0":"") + String(now.second());

  // Leer ADS1115
  voltCorriente1 = ads.readADC_SingleEnded(0) * Fc;
  voltCorriente2 = ads.readADC_SingleEnded(1) * Fc;
  voltVoltaje    = ads.readADC_SingleEnded(2) * Fc;

  // Corriente total
  corrienteTotal = voltCorriente1 + voltCorriente2;

  // LED2 ejemplo
  digitalWrite(led2, now.second()<10 ? HIGH : LOW);

  delay(500);
}
