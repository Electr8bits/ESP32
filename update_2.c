#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_ADS1X15.h>

// --- WiFi ---
const char* ssid = "MAJU 2.4G";
const char* password = "luhanaevaluna";

// --- Usuario y contraseña ---
const char* usuario = "admin";
const char* clave   = "chupetin";

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
const float Fc = 0.000125;
float voltCorriente1 = 0, voltCorriente2 = 0, voltVoltaje = 0;
float corrienteTotal = 0;

// --- Energía y costo ---
float energia_kWh = 0;       // energía acumulada en kWh
float costoSoles = 0;        // costo acumulado en soles
unsigned long ultimoMillis = 0;

// --- Estado de tarifa ---
String estadoTarifa = "";

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

  // Página principal con autenticación
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(usuario, clave)){
      return request->requestAuthentication(); // Pide usuario y contraseña
    }

    // HTML con CSS mejorado
    String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{font-family:Arial;background:#f2f2f2;margin:0;padding:0;text-align:center;}";
    html += "h1{color:#333;margin-top:20px;}";
    html += ".container{display:inline-block;background:#fff;padding:20px;margin:10px;border-radius:10px;box-shadow:0 4px 8px rgba(0,0,0,0.1);}";
    html += ".boton{padding:15px 30px;margin:10px;font-size:16px;border:none;border-radius:5px;cursor:pointer;}";
    html += ".on{background:#4CAF50;color:white;}";
    html += ".off{background:#f44336;color:white;}";
    html += ".medidas{font-size:15px;line-height:1.6;margin-top:15px;text-align:left;}";
    html += "</style>";
    html += "<script>";
    html += "function toggle(dev){";
    html += "fetch('/toggle?dev='+dev).then(r=>r.text()).then(t=>{";
    html += "document.getElementById(dev).innerHTML=t;";
    html += "document.getElementById(dev).className='boton '+(t.includes('ON')?'on':'off');});}";
    html += "setInterval(()=>{";
    html += "fetch('/hora').then(r=>r.text()).then(t=>{document.getElementById('hora').innerHTML=t;});";
    html += "fetch('/medidas').then(r=>r.text()).then(t=>{document.getElementById('medidas').innerHTML=t;});},1000);";
    html += "</script></head><body>";
    html += "<h1>Panel de Control</h1>";
    html += "<div class='container'>";
    html += "<h2>Hora RTC: <span id='hora'>--:--:--</span></h2>";
    html += "<div id='medidas' class='medidas'>Cargando medidas...</div><br>";
    html += "<button id='luz' class='boton " + String(estadoLuz ? "on":"off") + "' onclick=\"toggle('luz')\">Luz " + String(estadoLuz ? "ON":"OFF") + "</button><br>";
    html += "<button id='toma1' class='boton " + String(estadoToma1 ? "on":"off") + "' onclick=\"toggle('toma1')\">Tomacorriente 1 " + String(estadoToma1 ? "ON":"OFF") + "</button><br>";
    html += "<button id='toma2' class='boton " + String(estadoToma2 ? "on":"off") + "' onclick=\"toggle('toma2')\">Tomacorriente 2 " + String(estadoToma2 ? "ON":"OFF") + "</button><br>";
    html += "</div></body></html>";
    request->send(200, "text/html", html);
  });

  // Toggle salidas con autenticación
  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(usuario, clave)){
      return request->requestAuthentication();
    }

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
    if(!request->authenticate(usuario, clave)){ return request->requestAuthentication(); }
    request->send(200, "text/plain", horaRTC);
  });

  // Medidas ADS1115 + energía
  server.on("/medidas", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(usuario, clave)){ return request->requestAuthentication(); }
    String datos = "Corriente Toma1: " + String(voltCorriente1,2) + " A<br>";
    datos += "Corriente Toma2: " + String(voltCorriente2,2) + " A<br>";
    datos += "Corriente Total: " + String(corrienteTotal,2) + " A<br>";
    datos += "Voltaje: " + String(voltVoltaje,2) + " V<br>";
    datos += "<b>" + estadoTarifa + "</b><br>";
    datos += "Energia: " + String(energia_kWh,6) + " kWh<br>";
    datos += "Costo actual: S/ " + String(costoSoles,4) + "<br>";
    request->send(200, "text/html", datos);
  });

  server.begin();
}

void loop() {
  // Hora
  DateTime now = rtc.now();
  horaRTC = String(now.hour()) + ":" + (now.minute()<10?"0":"") + String(now.minute()) + ":" + (now.second()<10?"0":"") + String(now.second());

  // Leer ADS1115
  voltCorriente1 = ads.readADC_SingleEnded(0) * Fc; // A
  voltCorriente2 = ads.readADC_SingleEnded(1) * Fc; // A
  voltVoltaje    = ads.readADC_SingleEnded(2) * Fc; // V

  // Corriente total
  corrienteTotal = voltCorriente1 + voltCorriente2;

  // --- Cálculo energía y costo ---
  unsigned long ahoraMillis = millis();
  float dt_horas = (ahoraMillis - ultimoMillis) / 3600000.0; // delta t en horas
  ultimoMillis = ahoraMillis;

  // Potencia instantánea en W
  float potenciaW = voltVoltaje * corrienteTotal;
  // Acumular energía
  energia_kWh += (potenciaW * dt_horas) / 1000.0; // W*h → kWh

  // Precio según segundo actual + estado
  float precio;
  if (now.second() < 20) {
    precio = 0.5; // tarifa pico
    estadoTarifa = "VOLTAJE PICO ACTIVADO (ahorra)";
  } else {
    precio = 1.0; // normal
    estadoTarifa = "Voltaje pico desactivado (cobro normal)";
  }
  costoSoles = energia_kWh * precio;

  // LED2 ejemplo
  digitalWrite(led2, now.second()<20 ? HIGH : LOW);

  delay(500);
}
